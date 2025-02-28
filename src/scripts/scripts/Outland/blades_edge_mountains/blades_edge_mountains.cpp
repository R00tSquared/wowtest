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
SDName: Blades_Edge_Mountains
SD%Complete: 98
SDComment: Quest support: 10503, 10504, 10556, 10609, 10682, 10980, 10518, 10859, 10674, 11058 ,11080, 11059, 10675, 10867, 10557, 10710, 10711, 10712, 10821, 10911, 10723, 10802, 11000, 11026, 11051. Assault on Bash'ir Landing!. Ogri'la->Skettis Flight. (npc_daranelle needs bit more work before consider complete)
SDCategory: Blade's Edge Mountains
EndScriptData */

/* ContentData
mobs_nether_drake
npc_daranelle
npc_overseer_nuaar
npc_saikkal_the_elder
npc_skyguard_handler_irena
npc_bloodmaul_brutebane
npc_ogre_brute
npc_vim_bunny
npc_aether_ray
npc_wildlord_antelarion
npc_kolphis_darkscale
npc_prophecy_trigger
go_thunderspike
go_apexis_relic
go_simon_cluster
npc_simon_bunny
npc_orb_attracter
npc_razaan_event
npc_razaani_raide
npc_rally_zapnabber
npc_anger_camp
go_obeliks
npc_cannon_target
npc_gargrom
npc_soulgrinder
npc_bashir_landing
npc_banishing_crystal
EndContentData */

#include "precompiled.h"
#include "GameEvent.h"
#include <ctime>

/*######
## mobs_nether_drake
######*/

#define SAY_NIHIL_1                 -1000396
#define SAY_NIHIL_2                 -1000397
#define SAY_NIHIL_3                 -1000398
#define SAY_NIHIL_4                 -1000399
#define SAY_NIHIL_INTERRUPT         -1000400

#define ENTRY_WHELP                 20021
#define ENTRY_PROTO                 21821
#define ENTRY_ADOLE                 21817
#define ENTRY_MATUR                 21820
#define ENTRY_NIHIL                 21823

#define SPELL_T_PHASE_MODULATOR     37573

#define SPELL_ARCANE_BLAST          38881
#define SPELL_MANA_BURN             38884
#define SPELL_INTANGIBLE_PRESENCE   36513

struct mobs_nether_drakeAI : public ScriptedAI
{
    mobs_nether_drakeAI(Creature* creature) : ScriptedAI(creature) {}

    bool IsNihil;
    Timer NihilSpeech_Timer;
    Timer ArcaneBlast_Timer;
    Timer ManaBurn_Timer;
    Timer IntangiblePresence_Timer;
    uint32 NihilSpeech_Phase;

    void Reset()
    {
        NihilSpeech_Timer.Reset(2000);
        IsNihil = false;

        if (me->GetEntry() == ENTRY_NIHIL)
            IsNihil = true;

        NihilSpeech_Phase = 1;

        ArcaneBlast_Timer.Reset(7500);
        ManaBurn_Timer.Reset(10000);
        IntangiblePresence_Timer.Reset(15000);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_T_PHASE_MODULATOR && caster->GetTypeId() == TYPEID_PLAYER)
        {
            uint32 cEntry = 0;

            switch (me->GetEntry())
            {
                case ENTRY_WHELP:
                    if (rand() % 100 <= 10)
                        cEntry = ENTRY_NIHIL;
                    else 
                        cEntry = RAND(ENTRY_PROTO, ENTRY_ADOLE, ENTRY_MATUR);
                    break;
                case ENTRY_PROTO:
                    cEntry = RAND(ENTRY_ADOLE, ENTRY_MATUR);
                    break;
                case ENTRY_ADOLE:
                    cEntry = RAND(ENTRY_PROTO, ENTRY_MATUR);
                    break;
                case ENTRY_MATUR:
                    cEntry = RAND(ENTRY_PROTO, ENTRY_ADOLE);
                    break;
                case ENTRY_NIHIL:
                    if (NihilSpeech_Phase)
                    {
                        DoScriptText(SAY_NIHIL_INTERRUPT, me);
                        IsNihil = false;
                        cEntry = RAND(ENTRY_PROTO, ENTRY_ADOLE, ENTRY_MATUR);
                        NihilSpeech_Timer = 0;
                        NihilSpeech_Phase = 0;
                    }
                    break;
            }

            if (cEntry)
            {
                me->UpdateEntry(cEntry);

                if (cEntry == ENTRY_NIHIL)
                {
                    me->InterruptNonMeleeSpells(true);
                    me->RemoveAllAuras();
                    me->DeleteThreatList();
                    me->CombatStop(true);
                    Reset();
                }
                else
                {
                    me->AttackStop();
                    me->Attack(caster, true);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (IsNihil)
        {
            if (NihilSpeech_Phase)
            {
                if (NihilSpeech_Timer.Expired(diff))
                {
                    switch (NihilSpeech_Phase)
                    {
                        case 1:
                            DoScriptText(SAY_NIHIL_1, me);
                            ++NihilSpeech_Phase;
                            break;
                        case 2:
                            DoScriptText(SAY_NIHIL_2, me);
                            ++NihilSpeech_Phase;
                            break;
                        case 3:
                            DoScriptText(SAY_NIHIL_3, me);
                            ++NihilSpeech_Phase;
                            break;
                        case 4:
                            DoScriptText(SAY_NIHIL_4, me);
                            ++NihilSpeech_Phase;
                            break;
                        case 5:
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            me->SetLevitate(true);
                            //then take off to random location. creature is initially summoned, so don't bother do anything else.
                            me->GetMotionMaster()->MovePoint(0, me->GetPositionX()+100, me->GetPositionY(), me->GetPositionZ()+100);
                            NihilSpeech_Phase = 0;
                            break;
                    }
                    NihilSpeech_Timer = 5000;
                }
            }
            return;                                         //anything below here is not interesting for Nihil, so skip it
        }

        if (!UpdateVictim())
            return;

        if (IntangiblePresence_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_INTANGIBLE_PRESENCE);
            IntangiblePresence_Timer = 15000+rand()%15000;
        }


        if (ManaBurn_Timer.Expired(diff))
        {
            Unit* target = me->GetVictim();
            if (target && target->getPowerType() == POWER_MANA)
                DoCast(target,SPELL_MANA_BURN);
            ManaBurn_Timer = 8000+rand()%8000;
        }


        if (ArcaneBlast_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_ARCANE_BLAST);
            ArcaneBlast_Timer = 2500+rand()%5000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mobs_nether_drake(Creature* creature)
{
    return new mobs_nether_drakeAI (creature);
}

/*######
## npc_daranelle
######*/

#define SAY_DARANELLE -1000401

struct npc_daranelleAI : public ScriptedAI
{
    npc_daranelleAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (who->HasAura(36904,0))
            {
                DoScriptText(SAY_DARANELLE, me, who);
                //TODO: Move the below to updateAI and run if this statement == true
                ((Player*)who)->KilledMonster(21511, me->GetGUID());
                ((Player*)who)->RemoveAurasDueToSpell(36904);
            }
        }

        ScriptedAI::MoveInLineOfSight(who);
    }
};

CreatureAI* GetAI_npc_daranelle(Creature* creature)
{
    return new npc_daranelleAI (creature);
}

/*######
## npc_overseer_nuaar
######*/

#define GOSSIP_HON 16410

bool GossipHello_npc_overseer_nuaar(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(10682) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HON), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(10532, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_overseer_nuaar(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->SEND_GOSSIP_MENU(10533, creature->GetGUID());
        player->AreaExploredOrEventHappens(10682);
    }
    return true;
}

/*######
## npc_saikkal_the_elder
######*/

#define GOSSIP_HSTE 16411
#define GOSSIP_SSTE 16412

bool GossipHello_npc_saikkal_the_elder(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(10980) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HSTE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(10794, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_saikkal_the_elder(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SSTE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(10795, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
            player->SEND_GOSSIP_MENU(10796, creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_skyguard_handler_irena
######*/

#define GOSSIP_SKYGUARD 16413

bool GossipHello_npc_skyguard_handler_irena(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetReputationMgr().GetRank(1031) >= REP_HONORED)
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(GOSSIP_SKYGUARD), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_skyguard_handler_irena(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();

        std::vector<uint32> nodes;

        nodes.resize(2);
        nodes[0] = 172;                                     //from ogri'la
        nodes[1] = 171;                                     //end at skettis
        player->ActivateTaxiPathTo(nodes);                  //TaxiPath 706
    }
    return true;
}

/*######
## npc_bloodmaul_brutebane
######*/

enum eBloodmaul
{
    NPC_OGRE_BRUTE                       = 19995,
    NPC_QUEST_CREDIT                     = 21241,
    GO_KEG                               = 184315
};

struct npc_bloodmaul_brutebaneAI : public ScriptedAI
{
    npc_bloodmaul_brutebaneAI(Creature* creature) : ScriptedAI(creature){}

    void IsSummonedBy(Unit* pOwner)
    {
       if (Creature* pOgre = GetClosestCreatureWithEntry(me, NPC_OGRE_BRUTE, 50.0f))
       {
           pOgre->SetReactState(REACT_DEFENSIVE);
           pOgre->GetMotionMaster()->MovePoint(1, me->GetPositionX()-1, me->GetPositionY()+1, me->GetPositionZ());

           if (Player* plOwner = pOwner->GetCharmerOrOwnerPlayerOrPlayerItself())
               plOwner->KilledMonster(NPC_QUEST_CREDIT, pOgre->GetGUID());
       }
    }

    void UpdateAI(const uint32 uiDiff){}
};

CreatureAI* GetAI_npc_bloodmaul_brutebane(Creature* creature)
{
    return new npc_bloodmaul_brutebaneAI (creature);
}

/*######
## npc_ogre_brute
######*/

struct npc_ogre_bruteAI : public ScriptedAI
{
    npc_ogre_bruteAI(Creature* creature) : ScriptedAI(creature){}

    Timer EventTimer;
    Timer MoveHomeTimer;
    Timer WakeUpTimer;
    Timer PhraseTimer;
    Timer AttackTimer;
    uint32 Phase;
    uint32 AttackCounter;
    Timer ResetDanceTimer;

    void Reset()
    {
        if (me->GetDBTableGUIDLow() == 71422)
            EventTimer.Reset(10000);
        else EventTimer.Reset(0);
        Phase = 0;
        MoveHomeTimer.Reset(0);
        WakeUpTimer.Reset(0);
        PhraseTimer.Reset(0);
        AttackTimer.Reset(0);
        AttackCounter = 0;
        ResetDanceTimer.Reset(0);
    }

    void EnterCombat(Unit* who)
    {
        me->SetStandState(UNIT_STAND_STATE_STAND);
        EventTimer.Reset(0);
        Phase = 0;
        MoveHomeTimer.Reset(0);
        WakeUpTimer.Reset(0);
        PhraseTimer.Reset(0);
        AttackTimer.Reset(0);
        AttackCounter = 0;
        ResetDanceTimer.Reset(0);
    }
    void EventPulse(Unit* pUnit, uint32 number)
    {
        if (number == 1)
            ResetDanceTimer = 5000;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == 35040)
        {
            me->SetStandState(UNIT_STAND_STATE_DEAD);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_B, me, 0, 50);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 miscValue)
    {
        if (eventType == 5)
        {
            if (me->GetDBTableGUIDLow() == 71420)
            {
                me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_RANGED);
                me->CastSpell(sender, 35040, true);
                me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_D, me, 0, 100);
            }
        }
        if (eventType == 9)
        {
            if (me->GetDBTableGUIDLow() == 71422)
            {
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_B, me, 0, 100);
            }
        }
        if (eventType == 6)
        {
            if (me->GetDBTableGUIDLow() == 71423)
            {
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(101, 2498.201, 6553.22, -0.59);
            }
            if (me->GetDBTableGUIDLow() == 71420)
            {
                me->Say("O-o...", LANG_UNIVERSAL, 0);
            }
        }
        if (eventType == 8)
        {
            if (me->GetDBTableGUIDLow() == 71420)
            {
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                invoker->SetRooted(false);
                WakeUpTimer = 8000;
            }
            if (me->GetDBTableGUIDLow() == 71422)
            {
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetWalk(true);
                me->GetMotionMaster()->MoveTargetedHome();
                EventTimer = 20000;
            }
        }
        if (eventType == 10)
        {
            if (me->GetDBTableGUIDLow() != 21422 && Phase != 2)
            {
                std::list<Creature*> shamans = FindAllCreaturesWithEntry(19998, 40);
                for (std::list<Creature*>::iterator itr = shamans.begin(); itr != shamans.end(); ++itr)
                {
                    (*itr)->HandleEmote(RAND(11, 15, 53));
                }
                me->HandleEmote(RAND(15, 11, 53));
            }
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (id == 100)
        {
            me->SetFacingTo(4.6);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 0, 100);
        }

        if (id == 101)
        {
            me->SetRooted(true);
            me->CastSpell(me, 33423, false);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_C, me, 1500, 100);
            MoveHomeTimer = 5000;
        }

        if (id == 105)
        {
            me->SetFacingTo(1.4);
            AttackTimer = 1000;
        }

        if (type != POINT_MOTION_TYPE || id != 1)
            return;

        if (GameObject* pKeg = FindGameObject(GO_KEG, 20.0, me))
            pKeg->Delete();

        me->HandleEmoteCommand(7);
        me->SetReactState(REACT_AGGRESSIVE);

        if (!me->GetVictim())
            me->GetMotionMaster()->MoveTargetedHome();
        else
            me->GetMotionMaster()->MoveChase(me->GetVictim());
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (ResetDanceTimer.Expired(diff))
            {
                me->HandleEmote(0);
                ResetDanceTimer = 0;
            }

            if (AttackTimer.Expired(diff))
            {
                if (AttackCounter <= 7)
                {
                    me->CastSpell(me, RAND(33423, 33424, 33425), false);
                    me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_E, me, 1000, 50);
                    AttackTimer = 2500;
                    AttackCounter++;
                }
                else
                {
                    MoveHomeTimer = 1000;
                    EventTimer = 7000;
                    AttackTimer = 0;
                }
            }
            if (WakeUpTimer.Expired(diff))
            {
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetWalk(true);
                me->GetMotionMaster()->MoveTargetedHome();
                WakeUpTimer = 0;
            }

            if (MoveHomeTimer.Expired(diff))
            {
                me->GetMotionMaster()->MoveTargetedHome();
                MoveHomeTimer = 0;
            }

            if (EventTimer.Expired(diff))
            {
                switch (Phase)
                {
                case 0:
                    me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(100, 2503.738, 6593.100, 0.72);
                    EventTimer = 60000;
                    Phase = 1;
                    break;
                case 1:
                    // move same position, attack head, make others applaud, move home
                    me->GetMotionMaster()->MovePoint(105, 2503.738, 6593.100, 0.72);
                    EventTimer = 0;
                    Phase = 2;
                    break;
                case 2:
                    // find shaman, cast channeling, reset event
                    std::list<Creature*> shamans = FindAllCreaturesWithEntry(19998, 40);
                    for (std::list<Creature*>::iterator itr = shamans.begin(); itr != shamans.end(); ++itr)
                    {
                        if (Creature* trigger = GetClosestCreatureWithEntry(me, 20230, 100))
                            (*itr)->CastSpell(trigger, 35245, false);
                    }
                    me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_E, me, 2000, 50);
                    EventTimer = 30000;
                    Phase = 0;
                    break;
                }
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool ReceiveEmote_npc_ogre_brute(Player* plr, Creature* crt, uint32 emote)
{
    if (plr->GetReputationMgr().GetRank(1038) >= REP_NEUTRAL)
    {
        if (emote == TEXTEMOTE_KISS)
            crt->Say(-1200462, LANG_UNIVERSAL, 0);
        if (emote == TEXTEMOTE_WAVE)
            crt->HandleEmote(EMOTE_ONESHOT_WAVE);
        if (emote == TEXTEMOTE_FART)
            crt->Say(-1200463, LANG_UNIVERSAL, 0);
        if (emote == TEXTEMOTE_DANCE)
        {
            crt->HandleEmote(EMOTE_ONESHOT_DANCE);
            crt->AI()->EventPulse(crt, 1);
        }
    }
    return true;
}

CreatureAI* GetAI_npc_ogre_brute(Creature* creature)
{
    return new npc_ogre_bruteAI(creature);
}

/*######
## npc_vimgol_the_vile
######*/

#define SPELL_PENTAGRAM 39921
#define GO_FLAME_CIRCLE 185555
#define PENTAGRAM_TRIGGER 23040
#define MAIN_SPAWN 22911

#define VIMGOLE_VILLE_TEXT_1    -1043018
#define VIMGOLE_VILLE_TEXT_2    -1043019

struct npc_vimgol_the_vileAI : public ScriptedAI
{
    npc_vimgol_the_vileAI(Creature* creature) : ScriptedAI(creature)
    {
        SpawnEvent.Reset(1);
    }

    Timer SpawnEvent;
    Timer CheckTimer;
    bool isLowHp;

    void Reset()
    {
        isLowHp = false;
        CheckTimer.Reset(0);
    }

    void JustDied(Unit *pWho)
    {
        pWho->CastSpell(pWho, 39862, true);

        std::list<Creature*> triggers = FindAllCreaturesWithEntry(PENTAGRAM_TRIGGER, 50.0);
        for(std::list<Creature*>::iterator itr = triggers.begin(); itr != triggers.end(); ++itr)
            (*itr)->ForcedDespawn(1);
    }
    void JustReachedHome()
    {
        if (isLowHp)
        {
            DoScriptText(VIMGOLE_VILLE_TEXT_2, m_creature);
            m_creature->CastSpell(m_creature, 40545, false);
            CheckTimer.Reset(1000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (SpawnEvent.Expired(diff))
        {
            DoScriptText(VIMGOLE_VILLE_TEXT_1, m_creature);
            m_creature->CastSpell(m_creature, 7741, false);

            SpawnEvent = 0;
        }

        if (CheckTimer.Expired(diff))
        {
            std::list<Creature*> triggers = FindAllCreaturesWithEntry(PENTAGRAM_TRIGGER, 50.0);
            if (triggers.size() >= 5 && m_creature->IsNonMeleeSpellCast(false))
            {
                m_creature->InterruptNonMeleeSpells(true);
                CheckTimer = 0;
                return;
            }
            CheckTimer = 1000;
        }

        if (!UpdateVictim())
            return;
        
        if (m_creature->HealthBelowPct(50))
        {
            if (!isLowHp)
            {
                m_creature->GetMotionMaster()->MoveTargetedHome();
                isLowHp = true;
            }
        }

        DoMeleeAttackIfReady();

    }
};

CreatureAI* GetAI_npc_vimgol_the_vile(Creature* creature)
{
    return new npc_vimgol_the_vileAI (creature);
}

/*######
## npc_vim_bunny
######*/

struct npc_vim_bunnyAI : public ScriptedAI
{
    npc_vim_bunnyAI(Creature* creature) : ScriptedAI(creature){}

    Timer_UnCheked CheckTimer;

    void Reset()
    {
        CheckTimer.Reset(1000);
    }

    bool GetPlayer()
    {
        Player* pPlayer = NULL;
        Hellground::AnyPlayerInObjectRangeCheck p_check(me, 3.0f);
        Hellground::ObjectSearcher<Player, Hellground::AnyPlayerInObjectRangeCheck> searcher(pPlayer, p_check);

        Cell::VisitAllObjects(me, searcher, 3.0f);
        return pPlayer;
    }

    void UpdateAI(const uint32 diff)
    {
        if (CheckTimer.Expired(diff))
        {
            if (me->GetDistance2d(3279.80f, 4639.76f) < 5.0)
            {
                if (GetClosestCreatureWithEntry(me, MAIN_SPAWN, 80.0f))
                {
                    CheckTimer = 2000;
                    return;
                }

                // WE NEED HERE TO BE SURE THAT SPAWN IS VALID !
                std::list<Creature*> triggers = FindAllCreaturesWithEntry(PENTAGRAM_TRIGGER, 50.0);
                if (triggers.size() >= 5)
                {
                    DoSpawnCreature(MAIN_SPAWN,0,0,0,0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);

                    CheckTimer = 2000;
                    return;
                }

                CheckTimer = 2000;
            }
            else
            {
                if (GetPlayer())
                {
                    Unit* temp = DoSpawnCreature(PENTAGRAM_TRIGGER,0,0,2.0,0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                    temp->CastSpell(temp, SPELL_PENTAGRAM, false);
                    CheckTimer = 16000;
                    return;
                }
                CheckTimer = 2000;
            }
        }
    }
};

CreatureAI* GetAI_npc_vim_bunny(Creature* creature)
{
    return new npc_vim_bunnyAI (creature);
}

/*######
## Wrangle Some Aether Rays
######*/

// Spells
#define EMOTE_WEAK              -1200464
#define SPELL_SUMMON_WRANGLED   40917
#define SPELL_CHANNEL           40626

struct mob_aetherrayAI : public ScriptedAI
{
    mob_aetherrayAI(Creature* creature) : ScriptedAI(creature) {}

    bool Weak;
    uint64 PlayerGUID;

    void Reset()
    {
        Weak = false;
    }

    void EnterCombat(Unit* who)
    {
        if (Player* player = who->GetCharmerOrOwnerPlayerOrPlayerItself())
            PlayerGUID = player->GetGUID();
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->GetMotionMaster()->MoveFollow(Unit::GetPlayerInWorld(PlayerGUID), PET_FOLLOW_DIST, me->GetFollowAngle());
    }

    void UpdateAI(const uint32 diff)
    {

    if (!UpdateVictim())
            return;

    if (PlayerGUID) // start: support for quest 11066 and 11065
        {
            Player* target = Unit::GetPlayerInWorld(PlayerGUID);

            if (target && !Weak && me->GetHealth() < (me->GetMaxHealth() / 100 *40)
                && (target->GetQuestStatus(11066) == QUEST_STATUS_INCOMPLETE || target->GetQuestStatus(11065) == QUEST_STATUS_INCOMPLETE))
            {
                me->MonsterTextEmote(-1200464, 0, false);
                Weak = true;
            }

            if (Weak && me->HasAura(40856, 0))
            {
                me->CastSpell(target, SPELL_SUMMON_WRANGLED, false);
                target->KilledMonster(23343, me->GetGUID());
                me->AttackStop(); // delete the normal mob
                me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                me->RemoveCorpse();
            }
        }

        DoMeleeAttackIfReady();
    }


};

CreatureAI* GetAI_mob_aetherray(Creature* creature)
{
    return new mob_aetherrayAI (creature);
}

/*####
# npc_wildlord_antelarion
####*/

#define GOSSIP_ITEM_WILDLORD  16414
#define GOSSIP_ITEM2_WILDLORD 16415

bool GossipHello_npc_wildlord_antelarion(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
    {
        player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(10819) || player->GetQuestStatus(10820) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(31366,1))
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_WILDLORD), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO);

         if (player->GetQuestStatus(10910) || player->GetQuestStatus(10910) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(31763,1))
             player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM2_WILDLORD), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO+1);

        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_wildlord_antelarion(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    uint32 entry = 0;
    if (action == GOSSIP_SENDER_INFO)
        entry = 31366;
    else if (action == GOSSIP_SENDER_INFO +1)
        entry = 31763;

    if (entry != 0)
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, 1);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, entry, true);
            player->SendNewItem(item,1,true,false,true);
        }
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

/*########
# npc_kolphis_darkscale
#########*/

#define GOSSIP_ITEM_KOLPHIS1 16416
#define GOSSIP_ITEM_KOLPHIS2 16417
#define GOSSIP_ITEM_KOLPHIS3 16418
#define GOSSIP_ITEM_KOLPHIS4 16419
#define GOSSIP_ITEM_KOLPHIS5 16420

bool GossipHello_npc_kolphis_darkscale(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());
    if (player->GetQuestStatus(10722) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KOLPHIS1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(25019, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_kolphis_darkscale(Player* player, Creature* creature, uint32 sender, uint32 action)
{
  switch (action)
  {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KOLPHIS2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(25020, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KOLPHIS3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(25021, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KOLPHIS4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(25022, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KOLPHIS5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(25023, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->CompleteQuest(10722);
            player->SEND_GOSSIP_MENU(25024, creature->GetGUID());
            break;
  }
    return true;
}

/*#########
# npc_prophecy_trigger
#########*/

struct npc_prophecy_triggerAI : public ScriptedAI
{
    npc_prophecy_triggerAI(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (Player* plWho = pWho->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            if (plWho->GetQuestStatus(10607) == QUEST_STATUS_INCOMPLETE && plWho->HasAura(37466,0) && plWho->GetDistance(me) < 20.0f)
            {
                switch  (me->GetEntry())
                {
                    case 22798:
                        me->Whisper(-1200465, plWho->GetGUID());
                        break;
                    case 22799:
                        me->Whisper(-1200466, plWho->GetGUID());
                        break;
                    case 22800:
                        me->Whisper(-1200467, plWho->GetGUID());
                        break;
                    case 22801:
                        me->Whisper(-1200468, plWho->GetGUID());
                        break;
                }

                plWho->KilledMonster(me->GetEntry(), me->GetGUID());
                me->DisappearAndDie();
            }
        }
    }
};

CreatureAI* GetAI_npc_prophecy_trigger(Creature* creature)
{
    return new npc_prophecy_triggerAI(creature);
}

/*#########
# go_thunderspike
# UPDATE `gameobject_template` SET `ScriptName` = "go_thunderspike" WHERE `entry` = 184729;
#########*/

#define Q_THE_THUNDERSPIKE 10526
#define GOR_GRIMGUT_ENTRY  21319

bool GOUse_go_thunderspike(Player* player, GameObject* go)
{
    if (player->GetQuestStatus(Q_THE_THUNDERSPIKE) == QUEST_STATUS_INCOMPLETE)
    {
        // to prevent spawn spam :)
        if (Creature* pGor = GetClosestCreatureWithEntry(player, GOR_GRIMGUT_ENTRY, 50.0f))
        {
            if (!pGor->GetVictim() && pGor->isAlive())
                pGor->AI()->AttackStart(player);

            return false;
        }

        player->SummonCreature(GOR_GRIMGUT_ENTRY, 1315.86, 6686.35, -18.57, 0.02f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
    }

    return false;
}

/*######
## Simon Game - An Apexis Relic
######*/

enum SimonGame
{
    NPC_SIMON_BUNNY                 = 22923,
    NPC_APEXIS_GUARDIAN             = 22275,

    GO_APEXIS_RELIC                 = 185890,
    GO_APEXIS_MONUMENT              = 185944,
    GO_AURA_BLUE                    = 185872,
    GO_AURA_GREEN                   = 185873,
    GO_AURA_RED                     = 185874,
    GO_AURA_YELLOW                  = 185875,

    GO_BLUE_CLUSTER_DISPLAY         = 7369,
    GO_GREEN_CLUSTER_DISPLAY        = 7371,
    GO_RED_CLUSTER_DISPLAY          = 7373,
    GO_YELLOW_CLUSTER_DISPLAY       = 7375,
    GO_BLUE_CLUSTER_DISPLAY_LARGE   = 7364,
    GO_GREEN_CLUSTER_DISPLAY_LARGE  = 7365,
    GO_RED_CLUSTER_DISPLAY_LARGE    = 7366,
    GO_YELLOW_CLUSTER_DISPLAY_LARGE = 7367,

    SPELL_PRE_GAME_BLUE             = 40176,
    SPELL_PRE_GAME_GREEN            = 40177,
    SPELL_PRE_GAME_RED              = 40178,
    SPELL_PRE_GAME_YELLOW           = 40179,
    SPELL_VISUAL_BLUE               = 40244,
    SPELL_VISUAL_GREEN              = 40245,
    SPELL_VISUAL_RED                = 40246,
    SPELL_VISUAL_YELLOW             = 40247,

    SOUND_BLUE                      = 11588,
    SOUND_GREEN                     = 11589,
    SOUND_RED                       = 11590,
    SOUND_YELLOW                    = 11591,
    SOUND_DISABLE_NODE              = 11758,

    SPELL_AUDIBLE_GAME_TICK         = 40391,
    SPELL_VISUAL_START_PLAYER_LEVEL = 40436,
    SPELL_VISUAL_START_AI_LEVEL     = 40387,

    SPELL_BAD_PRESS_TRIGGER         = 41241,
    SPELL_BAD_PRESS_DAMAGE          = 40065,
    SPELL_REWARD_BUFF_1             = 40310,
    SPELL_REWARD_BUFF_2             = 40311,
    SPELL_REWARD_BUFF_3             = 40312,
};

enum SimonEvents
{
    EVENT_SIMON_SETUP_PRE_GAME         = 1,
    EVENT_SIMON_PLAY_SEQUENCE          = 2,
    EVENT_SIMON_RESET_CLUSTERS         = 3,
    EVENT_SIMON_PERIODIC_PLAYER_CHECK  = 4,
    EVENT_SIMON_TOO_LONG_TIME          = 5,
    EVENT_SIMON_GAME_TICK              = 6,
    EVENT_SIMON_ROUND_FINISHED         = 7,

    ACTION_SIMON_CORRECT_FULL_SEQUENCE = 8,
    ACTION_SIMON_WRONG_SEQUENCE        = 9,
    ACTION_SIMON_ROUND_FINISHED        = 10,
};

enum SimonColors
{
    SIMON_BLUE          = 0,
    SIMON_RED           = 1,
    SIMON_GREEN         = 2,
    SIMON_YELLOW        = 3,
    SIMON_MAX_COLORS    = 4,
};

struct npc_simon_bunnyAI : public ScriptedAI
{
    npc_simon_bunnyAI(Creature* creature) : ScriptedAI(creature) { }

    bool large;
    bool listening;
    uint8 gameLevel;
    uint8 fails;
    uint8 gameTicks;
    uint64 playerGUID;
    uint32 clusterIds[SIMON_MAX_COLORS];
    float zCoordCorrection;
    float searchDistance;
    EventMap _events;
    std::list<uint8> colorSequence, playableSequence, playerSequence;

    void UpdateAI(const uint32 diff)
    {
        _events.Update(diff);
        switch (_events.ExecuteEvent())
        {
            case EVENT_SIMON_PERIODIC_PLAYER_CHECK:
                if (!CheckPlayer())
                    ResetNode();
                else
                    _events.ScheduleEvent(EVENT_SIMON_PERIODIC_PLAYER_CHECK, 2000);
                break;
            case EVENT_SIMON_SETUP_PRE_GAME:
                SetUpPreGame();
                _events.CancelEvent(EVENT_SIMON_GAME_TICK);
                _events.ScheduleEvent(EVENT_SIMON_PLAY_SEQUENCE, 1000);
                break;
            case EVENT_SIMON_PLAY_SEQUENCE:
                if (!playableSequence.empty())
                {
                    PlayNextColor();
                    _events.ScheduleEvent(EVENT_SIMON_PLAY_SEQUENCE, 1500);
                }
                else
                {
                    listening = true;
                    AddSpellToCast(SPELL_VISUAL_START_PLAYER_LEVEL, CAST_NULL);
                    playerSequence.clear();
                    PrepareClusters();
                    gameTicks = 0;
                    _events.ScheduleEvent(EVENT_SIMON_GAME_TICK, 3000);
                }
                break;
            case EVENT_SIMON_GAME_TICK:
                AddSpellToCast(SPELL_AUDIBLE_GAME_TICK, CAST_NULL);
                
                if (gameTicks > gameLevel)
                    _events.ScheduleEvent(EVENT_SIMON_TOO_LONG_TIME, 500);
                else
                    _events.ScheduleEvent(EVENT_SIMON_GAME_TICK, 3000);
                gameTicks++;
                break;
            case EVENT_SIMON_RESET_CLUSTERS:
                PrepareClusters(true);
                break;
            case EVENT_SIMON_TOO_LONG_TIME:
                DoAction(ACTION_SIMON_WRONG_SEQUENCE);
                break;
            case EVENT_SIMON_ROUND_FINISHED:
                DoAction(ACTION_SIMON_ROUND_FINISHED);
                break;
        }
        CastNextSpellIfAnyAndReady();
    }

    void DoAction(const int32 action)
    {
        switch (action)
        {
            case ACTION_SIMON_ROUND_FINISHED:
                listening = false;
                AddSpellToCast(SPELL_VISUAL_START_AI_LEVEL, CAST_NULL);
                GiveRewardForLevel(gameLevel);
                _events.CancelEventsByGCD(0);
                if (gameLevel == 10)
                    ResetNode();
                else
                    _events.ScheduleEvent(EVENT_SIMON_SETUP_PRE_GAME, 1000);
                break;
            case ACTION_SIMON_CORRECT_FULL_SEQUENCE:
                gameLevel++;
                DoAction(ACTION_SIMON_ROUND_FINISHED);
                break;
            case ACTION_SIMON_WRONG_SEQUENCE:
                GivePunishment();
                DoAction(ACTION_SIMON_ROUND_FINISHED);
                break;
        }
    }

    // Called by color clusters script (go_simon_cluster) and used for knowing the button pressed by player
    void SetData(uint32 type, uint32 /*data*/)
    {
        if (!listening)
            return;

        uint8 pressedColor;

        if (type == clusterIds[SIMON_RED])
            pressedColor = SIMON_RED;
        else if (type == clusterIds[SIMON_BLUE])
            pressedColor = SIMON_BLUE;
        else if (type == clusterIds[SIMON_GREEN])
            pressedColor = SIMON_GREEN;
        else if (type == clusterIds[SIMON_YELLOW])
            pressedColor = SIMON_YELLOW;

        PlayColor(pressedColor);
        playerSequence.push_back(pressedColor);
        _events.ScheduleEvent(EVENT_SIMON_RESET_CLUSTERS, 500);
        CheckPlayerSequence();
    }

    // Used for getting involved player guid. Parameter id is used for defining if is a large(Monument) or small(Relic) node
    void SetGUID(uint64 guid, int32 id)
    {
        ClearCastQueue();

        me->SetLevitate(true);

        large = (bool)id;
        playerGUID = guid;
        StartGame();
    }

    /*
    Resets all variables and also find the ids of the four closests color clusters, since every simon
    node have diferent ids for clusters this is absolutely NECESSARY.
    */
    void StartGame()
    {
        listening = false;
        gameLevel = 0;
        fails = 0;
        gameTicks = 0;
        zCoordCorrection = large ? 8.0f : 2.75f;
        searchDistance = large ? 13.0f : 5.0f;
        colorSequence.clear();
        playableSequence.clear();
        playerSequence.clear();
        me->SetFloatValue(OBJECT_FIELD_SCALE_X, large ? 2 : 1);

        std::list<GameObject*> ClusterList;
        Hellground::AllGameObjectsInRange objects(me, searchDistance);
        Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsInRange> searcher(ClusterList, objects);
        Cell::VisitGridObjects(me, searcher, searchDistance);

        for (std::list<GameObject*>::const_iterator i = ClusterList.begin(); i != ClusterList.end(); ++i)
        {
            if (GameObject* go = (*i)->ToGameObject())
            {
                // We are checking for displayid because all simon nodes have 4 clusters with different entries
                if (large)
                {
                    switch (go->GetGOInfo()->displayId)
                    {
                        case GO_BLUE_CLUSTER_DISPLAY_LARGE: clusterIds[SIMON_BLUE] = go->GetEntry(); break;
                        case GO_RED_CLUSTER_DISPLAY_LARGE: clusterIds[SIMON_RED] = go->GetEntry(); break;
                        case GO_GREEN_CLUSTER_DISPLAY_LARGE: clusterIds[SIMON_GREEN] = go->GetEntry(); break;
                        case GO_YELLOW_CLUSTER_DISPLAY_LARGE: clusterIds[SIMON_YELLOW] = go->GetEntry(); break;
                    }
                }
                else
                {
                    switch (go->GetGOInfo()->displayId)
                    {
                        case GO_BLUE_CLUSTER_DISPLAY: clusterIds[SIMON_BLUE] = go->GetEntry(); break;
                        case GO_RED_CLUSTER_DISPLAY: clusterIds[SIMON_RED] = go->GetEntry(); break;
                        case GO_GREEN_CLUSTER_DISPLAY: clusterIds[SIMON_GREEN] = go->GetEntry(); break;
                        case GO_YELLOW_CLUSTER_DISPLAY: clusterIds[SIMON_YELLOW] = go->GetEntry(); break;
                    }
                }
            }
        }

        _events.Reset();
        _events.ScheduleEvent(EVENT_SIMON_ROUND_FINISHED, 1000);
        _events.ScheduleEvent(EVENT_SIMON_PERIODIC_PLAYER_CHECK, 2000);

        if (GameObject* relic = GetClosestGameObjectWithEntry(me, large ? GO_APEXIS_MONUMENT : GO_APEXIS_RELIC, searchDistance))
            relic->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
    }

    // Called when despawning the bunny. Sets all the node GOs to their default states.
    void ResetNode()
    {
        DoPlaySoundToSet(me, SOUND_DISABLE_NODE);

        for (uint32 clusterId = SIMON_BLUE; clusterId < SIMON_MAX_COLORS; clusterId++)
            if (GameObject* cluster = GetClosestGameObjectWithEntry(me, clusterIds[clusterId], searchDistance))
                cluster->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);

        for (uint32 auraId = GO_AURA_BLUE; auraId <= GO_AURA_YELLOW; auraId++)
            if (GameObject* auraGo = GetClosestGameObjectWithEntry(me, auraId, searchDistance))
                auraGo->RemoveFromWorld();

        if (GameObject* relic = GetClosestGameObjectWithEntry(me, large ? GO_APEXIS_MONUMENT : GO_APEXIS_RELIC, searchDistance))
            relic->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);

        me->ForcedDespawn(1000);
    }

    /*
    Called on every button click of player. Adds the clicked color to the player created sequence and
    checks if it corresponds to the AI created sequence. If so, incremente gameLevel and start a new
    round, if not, give punishment and restart current level.
    */
    void CheckPlayerSequence()
    {
        bool correct = true;
        if (playerSequence.size() <= colorSequence.size())
            for (std::list<uint8>::const_iterator i = playerSequence.begin(), j = colorSequence.begin(); i != playerSequence.end(); ++i, ++j)
                if ((*i) != (*j))
                    correct = false;

        if (correct && (playerSequence.size() == colorSequence.size()))
            DoAction(ACTION_SIMON_CORRECT_FULL_SEQUENCE);
        else if (!correct)
            DoAction(ACTION_SIMON_WRONG_SEQUENCE);
    }

    /*
    Generates a random sequence of colors depending on the gameLevel. We also copy this sequence to
    the playableSequence wich will be used when playing the sequence to the player.
    */
    void GenerateColorSequence()
    {
        colorSequence.clear();
        for (uint8 i = 0; i <= gameLevel; i++)
            colorSequence.push_back(RAND(SIMON_BLUE, SIMON_RED, SIMON_GREEN, SIMON_YELLOW));

        for (std::list<uint8>::const_iterator i = colorSequence.begin(); i != colorSequence.end(); ++i)
            playableSequence.push_back(*i);
    }


    // Remove any existant glowing auras over clusters and set clusters ready for interating with them.
    void PrepareClusters(bool clustersOnly = false)
    {
        for (uint32 clusterId = SIMON_BLUE; clusterId < SIMON_MAX_COLORS; clusterId++)
            if (GameObject* cluster = GetClosestGameObjectWithEntry(me, clusterIds[clusterId], searchDistance))
                cluster->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);

        if (clustersOnly)
            return;

        for (uint32 auraId = GO_AURA_BLUE; auraId <= GO_AURA_YELLOW; auraId++)
            if (GameObject* auraGo = GetClosestGameObjectWithEntry(me, auraId, searchDistance))
                auraGo->RemoveFromWorld();
    }

    /*
    Called when AI is playing the sequence for player. We cast the visual spell and then remove the
    cast color from the casting sequence.
    */
    void PlayNextColor()
    {
        PlayColor(*playableSequence.begin());
        playableSequence.erase(playableSequence.begin());
    }

    // Casts a spell and plays a sound depending on parameter color.
    void PlayColor(uint8 color)
    {
        switch (color)
        {
            case SIMON_BLUE:
                AddSpellToCast(SPELL_VISUAL_BLUE, CAST_NULL);
                DoPlaySoundToSet(me, SOUND_BLUE);
                break;
            case SIMON_GREEN:
                AddSpellToCast(SPELL_VISUAL_GREEN, CAST_NULL);
                DoPlaySoundToSet(me, SOUND_GREEN);
                break;
            case SIMON_RED:
                AddSpellToCast(SPELL_VISUAL_RED, CAST_NULL);
                DoPlaySoundToSet(me, SOUND_RED);
                break;
            case SIMON_YELLOW:
                AddSpellToCast(SPELL_VISUAL_YELLOW, CAST_NULL);
                DoPlaySoundToSet(me, SOUND_YELLOW);
                break;
        }
    }

    /*
    Creates the transparent glowing auras on every cluster of this node.
    After calling this function bunny is teleported to the center of the node.
    */
    void SetUpPreGame()
    {
        for (uint32 clusterId = SIMON_BLUE; clusterId < SIMON_MAX_COLORS; clusterId++)
        {
            if (GameObject* cluster = GetClosestGameObjectWithEntry(me,clusterIds[clusterId], 2.0f*searchDistance))
            {
                cluster->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);

                // break since we don't need glowing auras for large clusters
                if (large)
                    break;

                float x, y, z, o = 0.0f;
                cluster->GetPosition(x, y, z);
                me->NearTeleportTo(x, y, z, o);

                uint32 preGameSpellId;
                if (cluster->GetEntry() == clusterIds[SIMON_RED])
                    preGameSpellId = SPELL_PRE_GAME_RED;
                else if (cluster->GetEntry() == clusterIds[SIMON_BLUE])
                    preGameSpellId = SPELL_PRE_GAME_BLUE;
                else if (cluster->GetEntry() == clusterIds[SIMON_GREEN])
                    preGameSpellId = SPELL_PRE_GAME_GREEN;
                else if (cluster->GetEntry() == clusterIds[SIMON_YELLOW])
                    preGameSpellId = SPELL_PRE_GAME_YELLOW;
                else
                    break;

                me->CastSpell(cluster, preGameSpellId, true);
            }
        }

        if (GameObject* relic = GetClosestGameObjectWithEntry(me, large ? GO_APEXIS_MONUMENT : GO_APEXIS_RELIC, searchDistance))
        {
            float x, y, z, o = 0.0f;
            relic->GetPosition(x, y, z);
            me->NearTeleportTo(x, y, z + zCoordCorrection, o);
        }

        GenerateColorSequence();
    }

    // Handles the spell rewards. The spells also have the QuestCompleteEffect, so quests credits are working.
    void GiveRewardForLevel(uint8 level)
    {
        uint32 rewSpell;
        switch (level)
        {
            case 6:
                if (large)
                    GivePunishment();
                else
                    rewSpell = SPELL_REWARD_BUFF_1;
                break;
            case 8:
                rewSpell = SPELL_REWARD_BUFF_2;
                break;
            case 10:
                rewSpell = SPELL_REWARD_BUFF_3;
                break;
            default:
                rewSpell = 0;
        }

        if (rewSpell)
            if (Player* player = me->GetPlayerInWorld(playerGUID))
                player->CastSpell(player, rewSpell, true);
    }

    /*
    Depending on the number of failed pushes for player the damage of the spell scales, so we first
    cast the spell on the target that hits for 50 and shows the visual and then forces the player
    to cast the damaging spell on it self with the modified basepoints.
    4 fails = death.
    On large nodes punishment and reward are the same, summoning the Apexis Guardian.
    */
    void GivePunishment()
    {
        if (large)
        {
            if (Player* player = me->GetPlayerInWorld(playerGUID))
                if (Creature* guardian = me->SummonCreature(NPC_APEXIS_GUARDIAN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() - zCoordCorrection, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000))
                    guardian->AI()->AttackStart(player);

            ResetNode();
        }
        else
        {
            fails++;

            if (Player* player = me->GetPlayerInWorld(playerGUID))
                DoCast(player, SPELL_BAD_PRESS_TRIGGER, true);

            if (fails >= 4)
                ResetNode();
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {
        // Cast SPELL_BAD_PRESS_DAMAGE with scaled basepoints when the visual hits the target.
        // Need Fix: When SPELL_BAD_PRESS_TRIGGER hits target it triggers spell SPELL_BAD_PRESS_DAMAGE by itself
        // so player gets damage equal to calculated damage  dbc basepoints for SPELL_BAD_PRESS_DAMAGE (~50)
        if (spell->Id == SPELL_BAD_PRESS_TRIGGER)
        {
            int32 bp = (int32)((float)(fails)*0.33f*target->GetMaxHealth());
            target->CastCustomSpell(target, SPELL_BAD_PRESS_DAMAGE, &bp, NULL, NULL, true);
        }
    }

    // Checks if player has already die or has get too far from the current node
    bool CheckPlayer()
    {
        if (Player* player = me->GetPlayerInWorld(playerGUID))
        {
            if (player->isDead())
                return false;

            if (player->GetDistance2d(me) >= 2.0f*searchDistance)
            {
                GivePunishment();
                return false;
            }
        }
        else
            return false;

        return true;
    }
};

CreatureAI* Get_npc_simon_bunnyAI(Creature* creature)
{
    return new npc_simon_bunnyAI(creature);
}

bool OnGossipHello_go_simon_cluster(Player* player, GameObject* go)
{
    if (Creature* bunny = GetClosestCreatureWithEntry(go, NPC_SIMON_BUNNY, 13.0f, true))
        CAST_AI(npc_simon_bunnyAI, bunny->AI())->SetData(go->GetEntry(), 0);

    player->CastSpell(player, go->GetGOInfo()->goober.spellId, true);
    go->AddUse();
    return true;
}

enum ApexisRelic
{
    QUEST_APEXIS                 = 11058,
    QUEST_EMANATION              = 11080,
    QUEST_GUARDIAN               = 11059,
    GOSSIP_TEXT_ID               = 10948,

    ITEM_APEXIS_SHARD            = 32569,
    SPELL_TAKE_REAGENTS_SOLO     = 41145,
    SPELL_TAKE_REAGENTS_GROUP    = 41146,
};

#define GOSSIP_ITEM_1 16421
#define GOSSIP_ITEM_2 16422

bool OnGossipHello_go_apexis_relic(Player* player, GameObject* go)
{
    bool large = (go->GetEntry() == GO_APEXIS_MONUMENT);

    if (player->HasItemCount(ITEM_APEXIS_SHARD, large ? 35 : 1)/*&& large ? player->GetQuestStatus(QUEST_GUARDIAN) == QUEST_STATUS_INCOMPLETE : (player->GetQuestStatus(QUEST_APEXIS) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_EMANATION) == QUEST_STATUS_INCOMPLETE)*/)
        player->ADD_GOSSIP_ITEM(0, large ? player->GetSession()->GetHellgroundString(GOSSIP_ITEM_2) : player->GetSession()->GetHellgroundString(GOSSIP_ITEM_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID, go->GetGUID());

    return true;
}

bool OnGossipSelect_go_apexis_relic(Player* player, GameObject* go, uint32 sender, uint32 action)
{
    bool large = (go->GetEntry() == GO_APEXIS_MONUMENT);

    if (player->HasItemCount(ITEM_APEXIS_SHARD, large ? 35 : 1))
    {
        player->CastSpell(player, large ? SPELL_TAKE_REAGENTS_GROUP : SPELL_TAKE_REAGENTS_SOLO, false);

        if (Creature* bunny = player->SummonCreature(NPC_SIMON_BUNNY, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0))
            CAST_AI(npc_simon_bunnyAI, bunny->AI())->SetGUID(player->GetGUID(), large);

        player->CLOSE_GOSSIP_MENU();
    } 
    return false;
}

/*#########
# Q 10674 && 10859
#########*/

enum Entries
{
    NPC_LIGHT_ORB         = 20635,
    NPC_QUEST_CREDIT2     = 21929,
};

struct AttractOrbs
{
    AttractOrbs(Creature* t) : totem(t) {}
    Creature* totem;

    void operator()(Creature* orb)
    {
        if (orb->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
            orb->GetMotionMaster()->MovePoint(0, totem->GetPositionX(), totem->GetPositionY(), totem->GetPositionZ(), true);

        if (orb->IsWithinDistInMap(totem, 2.0f))
        {
            orb->ForcedDespawn();
            KillCredit();
        }
    }

    void KillCredit()
    { 
        Map* map = totem->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();

        for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (Player* player = itr->getSource())
            {
                if (totem->IsWithinDistInMap(player, 15.0f) && (player->GetQuestStatus(10674) || player->GetQuestStatus(10859) == QUEST_STATUS_INCOMPLETE))
                    player->KilledMonster(NPC_QUEST_CREDIT2, totem->GetGUID());
            }
        }
    }
};

struct npc_orb_attracterAI : public Scripted_NoMovementAI
{
    npc_orb_attracterAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

    Timer attractTimer;

    void Reset()
    {
        me->SetReactState(REACT_PASSIVE);
        attractTimer.Reset(1500);
        me->ForcedDespawn(10000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (attractTimer.Expired(diff))
        {
            std::list<Creature*> orbs = FindAllCreaturesWithEntry(NPC_LIGHT_ORB, 35.0f);
            std::for_each(orbs.begin(), orbs.end(), AttractOrbs(me));

            attractTimer = 1000;
        }
    }
};

CreatureAI* Get_npc_orb_attracterAI(Creature* creature)
{
    return new npc_orb_attracterAI(creature);
}

/*######
## npc_razaan_event
######*/

enum
{
    QUEST_MERCY        = 10675,
    QUEST_RESPONSE     = 10867,

    GO_SOULS           = 185033,

    YELL_RAZAAN        = -1900225,

    NPC_RAZAAN         = 21057
};

struct npc_razaan_eventAI : public ScriptedAI
{
    npc_razaan_eventAI(Creature* creature) : ScriptedAI(creature) {}

    bool Check;

    Timer CheckTimer;
    uint32 Count;

    void Reset()
    {
        Check = false;
        CheckTimer.Reset(0);
        Count = 0;
    }

    void HeyYa()
    {
        ++Count;

        if (Count == 10)
        {
            me->SummonCreature(NPC_RAZAAN, me->GetPositionX()+2.9f, me->GetPositionY()-5.8f, me->GetPositionZ()-8.9f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
            Check = true;
            CheckTimer = 5000;
            Count = 0;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        DoScriptText(YELL_RAZAAN, summoned);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Check)
        {
            if (CheckTimer.Expired(diff))
            {
                if (Creature* razaan = GetClosestCreatureWithEntry(me, NPC_RAZAAN, 75.0f, true))
                {
                    CheckTimer = 5000;
                    return;
                }
                else
                {
                    if (Creature* razaan = GetClosestCreatureWithEntry(me, NPC_RAZAAN, 75.0f, false))
                        me->SummonGameObject(GO_SOULS, razaan->GetPositionX(), razaan->GetPositionY(), razaan->GetPositionZ() + 3.0f, razaan->GetOrientation(), 0, 0, 0, 0, 50);

                    Reset();
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_razaan_event(Creature* creature)
{
    return new npc_razaan_eventAI (creature);
}

/*######
## npc_razaani_raider
######*/

enum
{
    SPELL_FLARE        = 35922,
    SPELL_WARP         = 32920,

    NPC_EORB           = 21025,
    NPC_DEADSOUL       = 20845
};

struct npc_razaani_raiderAI : public ScriptedAI
{
    npc_razaani_raiderAI(Creature* creature) : ScriptedAI(creature) {}

    uint64 PlayerGUID;
    Timer_UnCheked FlareTimer;
    Timer_UnCheked WarpTimer;

    void Reset()
    {
        PlayerGUID = 0;
        FlareTimer.Reset(urand(4000, 8000));
        WarpTimer.Reset(urand(8000, 13000));
    }

    void AttackStart(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)who)->GetQuestStatus(QUEST_MERCY)== QUEST_STATUS_INCOMPLETE || ((Player*)who)->GetQuestStatus(QUEST_RESPONSE) == QUEST_STATUS_INCOMPLETE)
                PlayerGUID = who->GetGUID();
        }

        ScriptedAI::AttackStart(who);
    }

    void JustSummoned(Creature* summoned)
    {
        Map* tmpMap = me->GetMap();

        if (!tmpMap)
            return;

        if (Creature*  eorb = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_EORB)))
        {
            summoned->SetLevitate(true);
            summoned->GetMotionMaster()->MovePoint(0, eorb->GetPositionX(), eorb->GetPositionY(), eorb->GetPositionZ());
        }
    }

    void JustDied(Unit* who)
    {
        if (PlayerGUID != 0)
        {
            me->SummonCreature(NPC_DEADSOUL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()+3, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 20000);

            Map* tmpMap = me->GetMap();

            if (!tmpMap)
                return;

            if (Creature*  eorb = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_EORB)))
                CAST_AI(npc_razaan_eventAI, eorb->AI())->HeyYa();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FlareTimer.Expired(diff))
        {
            DoCast (me->GetVictim(), SPELL_FLARE);
            FlareTimer = urand(9000, 14000);
        }


        if (WarpTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_WARP);
            WarpTimer = urand(14000, 18000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_razaani_raider(Creature* creature)
{
    return new npc_razaani_raiderAI (creature);
}

/*######
## npc_rally_zapnabber
######*/

enum
{
    SPELL_10557              = 37910,
    SPELL_10710              = 37962,
    SPELL_10711              = 36812,
    SPELL_10712              = 37968,
    //SPELL_10716            = 37940,
    SPELL_CANNON_CHANNEL     = 36795,
    SPELL_PORT               = 38213,
    SPELL_CHARGED            = 37108,
    SPELL_STATE1             = 36790,
    SPELL_STATE2             = 36792,
    SPELL_STATE3             = 36800,

    QUEST_JAGGED             = 10557,
    QUEST_SINGING            = 10710,
    QUEST_RAZAAN             = 10711,
    QUEST_RUUAN              = 10712,

    ITEM_WAIVER_SIGNED       = 30539,

    NPC_CHANNELER            = 21393,
    NPC_CH_TARGET            = 21394
};

float Port[3] =
{
    1920.163,
    5581.826,
    269.222
};

struct npc_rally_zapnabberAI : public ScriptedAI
{
    npc_rally_zapnabberAI(Creature* creature) : ScriptedAI(creature) {}

    bool Flight;

    uint64 playerGUID;
    Timer_UnCheked FlightTimer;
    Timer_UnCheked EffectTimer;
    uint8 flights;
    uint8 Count;

    void Reset() 
    {
        Flight = false;
        playerGUID = 0;
        FlightTimer = 0;
        EffectTimer = 0;
        flights = 0;
        Count = 0;
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void TestFlight(uint64 guid, uint8 flight)
    {
        playerGUID = guid;
        flights = flight;

        if (Player* player = me->GetPlayerInWorld(playerGUID))
        {
            if (player->IsMounted())
            {
                player->Unmount();
                player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
            }

            player->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);

            switch  (flights)
            {
                 case 1:
                     DoTeleportPlayer(player, Port[0], Port[1], Port[2], 5.1f);
                     break;
                 case 2:
                     DoTeleportPlayer(player, Port[0], Port[1], Port[2], 1.1f);
                     break;
                 case 3:
                     DoTeleportPlayer(player, Port[0], Port[1], Port[2], 2.7f);
                     break;
                 case 4:
                     DoTeleportPlayer(player, Port[0], Port[1], Port[2], 3.0f);
                     break;
            }

            player->CastSpell(player, SPELL_PORT, true);

            Map* tmpMap = me->GetMap();

            if (!tmpMap)
                return;
 
            if (Creature*  channeler = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_CHANNELER)))
                channeler->CastSpell(channeler, SPELL_CANNON_CHANNEL, true);

            Flight = true;
            EffectTimer = 3000;
            FlightTimer = 10000;
        }
    }

    void Flights()
    {
        if (Player* player = me->GetPlayerInWorld(playerGUID))
        {
            switch  (flights)
            {
                case 1:
                    player->GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);
                    player->CastSpell(player, SPELL_10557, true);
                    player->CastSpell(player, SPELL_CHARGED, true);
                    Reset();
                    break;
                case 2:
                    player->GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);
                    player->CastSpell(player, SPELL_10710, true);
                    player->CastSpell(player, SPELL_CHARGED, true);
                    Reset();
                    break;
                case 3:
                    player->GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);
                    player->CastSpell(player, SPELL_10711, true);
                    player->CastSpell(player, SPELL_CHARGED, true);
                    Reset();
                    break;
                case 4:
                    player->GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);
                    player->CastSpell(player, SPELL_10712, true);
                    player->CastSpell(player, SPELL_CHARGED, true);
                    Reset();
                    break;
            }
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if (Flight)
        {
            if (EffectTimer.Expired(diff))
            {
                Map* tmpMap = me->GetMap();

                if (!tmpMap)
                    return;

                if (Creature*  target = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_CH_TARGET)))
                {
                    ++Count;

                    switch  (Count)
                    {
                        case 1:
                            target->CastSpell(target, SPELL_STATE1, true);
                            break;
                        case 2:
                            target->CastSpell(target, SPELL_STATE2, true);
                            break;
                        case 3:
                            target->CastSpell(target, SPELL_STATE3, true);
                            break;
                    }
                }

                EffectTimer = 3000;
 
            }


            if (FlightTimer.Expired(diff))
                Flights();
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_rally_zapnabber(Creature* creature)
{
    return new npc_rally_zapnabberAI (creature);
}

#define GOSSIP_ITEM_FLIGHT1  16423
#define GOSSIP_ITEM_FLIGHT2  16424
#define GOSSIP_ITEM_FLIGHT3  16425
#define GOSSIP_ITEM_FLIGHT4  16426
#define GOSSIP_ITEM_FLIGHT5  16427
#define GOSSIP_ITEM_FLIGHT6  16428
#define GOSSIP_ITEM_FLIGHT7  16429
#define GOSSIP_ITEM_FLIGHT8  16430
#define GOSSIP_ITEM_FLIGHT9  16431
#define GOSSIP_ITEM_FLIGHT10 16432

bool GossipHello_npc_rally_zapnabber(Player* player, Creature* creature)
{
    if (player->GetQuestStatus(QUEST_JAGGED) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_CHARGED))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (player->GetQuestStatus(QUEST_SINGING) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_CHARGED))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

    if (player->GetQuestStatus(QUEST_RAZAAN) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_CHARGED))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    if (player->GetQuestStatus(QUEST_RUUAN) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_CHARGED))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

    if ((player->GetQuestRewardStatus(QUEST_JAGGED) || player->GetQuestRewardStatus(QUEST_SINGING) || player->GetQuestRewardStatus(QUEST_RAZAAN) || player->GetQuestRewardStatus(QUEST_RUUAN)) && !player->HasAura(SPELL_CHARGED))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_rally_zapnabber(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    uint8 flight;
    flight = 0;

    switch  (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            flight = 1;
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            CAST_AI(npc_rally_zapnabberAI, creature->AI())->TestFlight(player->GetGUID(), flight);
            player->CLOSE_GOSSIP_MENU();
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            if (player->HasItemCount(ITEM_WAIVER_SIGNED, 1))
            {
                flight = 2;
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                CAST_AI(npc_rally_zapnabberAI, creature->AI())->TestFlight(player->GetGUID(), flight);
                player->CLOSE_GOSSIP_MENU();
            }
            else player->CLOSE_GOSSIP_MENU();
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            flight = 3;
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            CAST_AI(npc_rally_zapnabberAI, creature->AI())->TestFlight(player->GetGUID(), flight);
            player->CLOSE_GOSSIP_MENU();
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
            flight = 4;
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            CAST_AI(npc_rally_zapnabberAI, creature->AI())->TestFlight(player->GetGUID(), flight);
            player->CLOSE_GOSSIP_MENU();
            break;
        case GOSSIP_ACTION_INFO_DEF + 5:
            flight = 2;
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            CAST_AI(npc_rally_zapnabberAI, creature->AI())->TestFlight(player->GetGUID(), flight);
            player->CLOSE_GOSSIP_MENU();
            break;
        case GOSSIP_ACTION_INFO_DEF + 6:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(10561, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 7:
            if (player->GetQuestRewardStatus(QUEST_JAGGED) && !player->HasAura(SPELL_CHARGED))
                player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (player->GetQuestRewardStatus(QUEST_SINGING) && !player->HasAura(SPELL_CHARGED))
                player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT8), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

            if (player->GetQuestRewardStatus(QUEST_RAZAAN) && !player->HasAura(SPELL_CHARGED))
                player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT9), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

            if (player->GetQuestRewardStatus(QUEST_RUUAN) && !player->HasAura(SPELL_CHARGED))
                player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT10), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(10562, creature->GetGUID());
            break;
    }

    return true;
}

/*######
## Q 10821
######*/

enum
{
    SPELL_BEAM        = 35846,

    QUEST_FIRED       = 10821,

    NPC_ANGER         = 22422,
    NPC_INVISB        = 20736,
    NPC_DOOMCRYER     = 19963
};

struct npc_anger_campAI : public ScriptedAI
{
    npc_anger_campAI(Creature* creature) : ScriptedAI(creature) {}

    uint8 Count;

    void Reset() 
    {
        Count = 0;
    }

    void Activate()
    {
        ++Count;

        if (Count == 5)
        {
            me->SummonCreature(NPC_DOOMCRYER, me->GetPositionX()+26.8f, me->GetPositionY()+9.1f, me->GetPositionZ()-9.6f, me->GetOrientation()/2.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
            Reset();
        }
    }
};

CreatureAI* GetAI_npc_anger_camp(Creature* creature)
{
    return new npc_anger_campAI (creature);
}

bool GOUse_go_obeliks(Player* player, GameObject* go)
{
    if (player->GetQuestStatus(QUEST_FIRED) == QUEST_STATUS_INCOMPLETE)
    {
        Map* tmpMap = go->GetMap();

        if (!tmpMap)
            return true;

        if (Creature* anger = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_ANGER)))
        {
            if (Creature* bunny = go->SummonCreature(NPC_INVISB, go->GetPositionX()-2.0f, go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 181000))
            {
                bunny->CastSpell(anger, SPELL_BEAM, true);
                CAST_AI(npc_anger_campAI, anger->AI())->Activate();
            }
        }
    }
    return false;
}

/*######
## npc_cannon_target
######*/

enum
{
    SPELL_ARTILLERY      = 39221,
    SPELL_IMP_AURA       = 39227,
    SPELL_HOUND_AURA     = 39275,

    NPC_IMP              = 22474,
    NPC_HOUND            = 22500,
    NPC_SOUTH_GATE       = 22472,
    NPC_NORTH_GATE       = 22471,
    CREDIT_SOUTH         = 22504,
    CREDIT_NORTH         = 22503,

    GO_FIRE              = 185317,
    GO_BIG_FIRE          = 185319
};

struct npc_cannon_targetAI : public ScriptedAI
{
    npc_cannon_targetAI(Creature* creature) : ScriptedAI(creature) {}

    bool PartyTime;

    uint64 PlayerGUID;
    uint64 CannonGUID;
    Timer PartyTimer;
    uint8 Count;

    void Reset() 
    {
        PartyTime  = false;
        PlayerGUID = 0;
        CannonGUID = 0;
        PartyTimer = 0;
        Count = 0;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_ARTILLERY)
        {
            ++Count;

            if (Count == 1)
            {
                if (Player* player = caster->GetCharmerOrOwnerPlayerOrPlayerItself())
                    PlayerGUID = player->GetGUID();

                CannonGUID = caster->GetGUID();
                PartyTime = true;
                PartyTimer = 3000;
            }

            if (Count == 3)
                me->SummonGameObject(GO_FIRE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 130);

            if (Count == 7)
            {
                if (Player* player = me->GetPlayerInWorld(PlayerGUID))
                {
                    if (Creature* bunny = GetClosestCreatureWithEntry(me, NPC_SOUTH_GATE, 200.0f))
                        player->KilledMonster(CREDIT_SOUTH, me->GetGUID());
                    else
                    {   
                        if (Creature* bunny = GetClosestCreatureWithEntry(me, NPC_NORTH_GATE, 200.0f))
                            player->KilledMonster(CREDIT_NORTH, me->GetGUID());
                    }
                }

                me->SummonGameObject(GO_BIG_FIRE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 60);
                Reset();
            }
        }
        return;
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_IMP)
            summoned->CastSpell(summoned, SPELL_IMP_AURA, true);
        else
        {
            if (summoned->GetEntry() == NPC_HOUND)
                summoned->CastSpell(summoned, SPELL_HOUND_AURA, true);
        }

        if (Creature* cannon = me->GetCreature(CannonGUID))
            summoned->AI()->AttackStart(cannon); 
    }

    void UpdateAI(const uint32 diff)
    {
        if (PartyTime)
        {
            if (PartyTimer.Expired(diff))
            {
                if (Creature* cannon = me->GetCreature(CannonGUID))
                {
                    if (cannon->isDead())
                        Reset();
                }

                switch(me->GetDBTableGUIDLow())
                {
                    case 78948:
                        if (roll_chance_i(20))
                            me->SummonCreature(NPC_HOUND, 1971.729, 5308.59, 154.06, 3.744, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        else
                            me->SummonCreature(NPC_IMP, 1971.729, 5308.59, 154.06, 3.744, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        break;
                    case 78947:
                        if (roll_chance_i(20))
                            me->SummonCreature(NPC_HOUND, 2192.65, 5469.506, 153.578, 5.198, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        else
                            me->SummonCreature(NPC_IMP, 2192.65, 5469.506, 153.578, 5.198, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        break;
                }
                PartyTimer = 3000;
            }
        }
    }
};

CreatureAI* GetAI_npc_cannon_target(Creature* creature)
{
    return new npc_cannon_targetAI (creature);
}

/*######
## npc_gargrom
######*/

#define GO_TEMP    185232

struct Pos
{
    float x, y, z;
};

static Pos sum[]=
{
    {3610.64f, 7180.36f, 139.98f},
    {3611.35f, 7179.39f, 140.10f}
};

struct npc_gargromAI : public ScriptedAI
{
    npc_gargromAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset() 
    {
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(0, sum[0].x, sum[0].y, sum[0].z);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            me->setDeathState(JUST_DIED);
            me->SummonGameObject(GO_TEMP, sum[1].x, sum[1].y, sum[1].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 300);
        }
    }
};

CreatureAI* GetAI_npc_gargrom(Creature* creature)
{
    return new npc_gargromAI (creature);
}

/*######
## npc_soulgrinder
######*/

enum
{
    SPELL_VRITUAL            = 39918,
    SPELL_GTRANSFORM         = 39916,
    SPELL_RBEAM              = 39920,
    SPELL_SHADOWFORMONE      = 39943,
    SPELL_SHADOWFORMTWO      = 39944,
    SPELL_SHADOWFORMTHREE    = 39946,
    SPELL_SHADOWFORMFOUR     = 39947,

    NPC_SOULGRINGER          = 23019,
    NPC_SSPIRIT              = 22912,
    NPC_BRITUAL              = 23037,
    NPC_SCULLOC              = 22910,

    QUEST_SOULGRINGER        = 11000
};

struct Ev
{
    float x, y, z, o;
};

static Ev ent[]=
{
    {3493.81f, 5530.01f, 19.57f, 0.80f},
    {3466.89f, 5566.49f, 20.24f, 0.30f},
    {3486.58f, 5551.74f, 7.51f, 0.65f},
    {3515.08f, 5527.14f, 20.17f, 1.20f},
    {3470.48f, 5583.96f, 20.68f, 6.20f}
};

struct npc_soulgrinderAI : public ScriptedAI
{
    npc_soulgrinderAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

    bool DoSpawns;

    SummonList summons;
    Timer_UnCheked SpawnTimer;
    Timer_UnCheked BeamTimer;
    uint8 Count;
    uint64 PlayerGUID;
    uint64 SkullocGUID;

    void Reset() 
    {
        if (Creature* bm = GetClosestCreatureWithEntry(me, NPC_SOULGRINGER, 10.0f))
        {
            if (me->GetGUID() != bm->GetGUID())
                me->ForcedDespawn();
        }

        DoSpawns = true;
        SpawnTimer.Reset(5000);
        BeamTimer.Reset(35000);
        Count = 0;
        PlayerGUID = 0;
        SkullocGUID = 0;
        DoCast(me, SPELL_VRITUAL, true);
        INeedTarget();
    }

    void INeedTarget()
    { 
        Map* map = me->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();

        for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (Player* player = itr->getSource())
            {
                if (me->IsWithinDistInMap(player, 10.0f) && player->GetQuestStatus(QUEST_SOULGRINGER) == QUEST_STATUS_INCOMPLETE)
                    PlayerGUID = player->GetGUID();
            }
        }
    }

    void DoSpawn()
    {
        float fx, fy, fz;

        me->GetNearPoint(fx, fy, fz, 0.0f, 20.0f, 0.0f);
        me->SummonCreature(NPC_SSPIRIT, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
        me->GetNearPoint(fx, fy, fz, 0.0f, 20.0f, 4.6f);
        me->SummonCreature(NPC_SSPIRIT, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);

        if (roll_chance_i(60))
        {
            me->GetNearPoint(fx, fy, fz, 0.0f, 20.0f, 1.5f);
            me->SummonCreature(NPC_SSPIRIT, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
        }

        if (roll_chance_i(50))
        {
            me->GetNearPoint(fx, fy, fz, 0.0f, 20.0f, 3.1f);
            me->SummonCreature(NPC_SSPIRIT, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summons.Summon(summoned);

        if (summoned->GetEntry() == NPC_SSPIRIT)
        {
            summoned->CastSpell(summoned , SPELL_GTRANSFORM, true);

            if (Unit* target = me->GetPlayerInWorld(PlayerGUID))
                summoned->AI()->AttackStart(target);
        }

        if (summoned->GetEntry() == NPC_BRITUAL)
        {
            summoned->CastSpell(me, SPELL_RBEAM, true);
        }
        else 
        {
            if (summoned->GetEntry() == NPC_SCULLOC)
                SkullocGUID = summoned->GetGUID();
        }
    }

    void Beam()
    {
        ++Count;

        switch (Count)
        {
            case 1:
                DoCast(me, SPELL_SHADOWFORMONE, true);
                me->SummonCreature(NPC_BRITUAL, ent[0].x, ent[0].y, ent[0].z, ent[0].o, TEMPSUMMON_DEAD_DESPAWN, 5000);
                break;
            case 2:
                DoCast(me, SPELL_SHADOWFORMTWO, true);
                me->SummonCreature(NPC_BRITUAL, ent[1].x, ent[1].y, ent[1].z, ent[1].o, TEMPSUMMON_DEAD_DESPAWN, 5000);
                me->SummonCreature(NPC_SCULLOC, ent[2].x, ent[2].y, ent[2].z, ent[2].o, TEMPSUMMON_DEAD_DESPAWN, 5000);
                break;
            case 3:
                DoCast(me, SPELL_SHADOWFORMTHREE, true);
                me->SummonCreature(NPC_BRITUAL, ent[3].x, ent[3].y, ent[3].z, ent[3].o, TEMPSUMMON_DEAD_DESPAWN, 5000);
                break;
            case 4:
                DoCast(me, SPELL_SHADOWFORMFOUR, true);
                me->SummonCreature(NPC_BRITUAL, ent[4].x, ent[4].y, ent[4].z, ent[4].o, TEMPSUMMON_DEAD_DESPAWN, 5000);
                break;
            case 5:
                DoSpawns = false;
                summons.DespawnEntry(NPC_BRITUAL);

                if (Creature* Skulloc = me->GetCreature(SkullocGUID))
                    Skulloc->CastSpell(me, SPELL_RBEAM, true);
                break;
            case 6:
                summons.DespawnEntry(NPC_SSPIRIT);

                if (Creature* Skulloc = me->GetCreature(SkullocGUID))
                    Skulloc->SetVisibility(VISIBILITY_OFF);

                summons.DespawnEntry(NPC_SCULLOC);

                if (Creature* Skulloc = me->SummonCreature(NPC_SCULLOC, ent[2].x, ent[2].y, ent[2].z, ent[2].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000))
                {
                    Skulloc->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    if (Unit* target = me->GetPlayerInWorld(PlayerGUID))
                        Skulloc->AI()->AttackStart(target);
                }
                me->ForcedDespawn();
                break;
        }

    }

    void UpdateAI(const uint32 diff)
    {
        if (DoSpawns)
        {            
            if (SpawnTimer.Expired(diff))
            {
                DoSpawn();

                SpawnTimer = 20000;
            }
        }

        if (BeamTimer.Expired(diff))
        {
            Beam();

            if (Count == 5)
                BeamTimer = 5000;
            else BeamTimer = 80000;
        }
    }
};

CreatureAI* GetAI_npc_soulgrinder(Creature* creature)
{
    return new npc_soulgrinderAI (creature);
}

/*######
## Assault on Bash'ir Landing!
######*/

enum
{
    YELL_AETHER_1     = -1900226,
    YELL_AETHER_2     = -1900227,
    YELL_AETHER_3     = -1900230,
    YELL_AETHER_4     = -1900232,
    YELL_ASSISTANT    = -1900228,
    YELL_ADEPT        = -1900229,
    YELL_MASTER       = -1900231,

    NPC_SSLAVE        = 23246,
    NPC_FFIEND        = 23249,
    NPC_SUBPRIMAL     = 23247,
    NPC_CONTROLLER    = 23368,
    NPC_HARBRINGER    = 23390,
    NPC_INQUISITOR    = 23414,
    NPC_RECKONER      = 23332,
    NPC_GCOLLECTOR    = 23333,
    NPC_DISRUPTORT    = 23250,
    NPC_AETHER        = 23241,
    NPC_ASSISTANT     = 23243,
    NPC_ADEPT         = 23244,
    NPC_MASTER        = 23245,
    NPC_SRANGER       = 23242,
    NPC_LIEUTENANT    = 23430
};

struct Assault
{
    float x, y, z, o;
};

static Assault AssaultPos[]=
{
    { 4014.87f, 5891.27f, 267.870f, 0.608f },
    { 4018.21f, 5888.22f, 267.870f, 1.271f },
    { 4013.54f, 5895.92f, 267.870f, 6.138f },
    { 2529.85f, 7323.99f, 375.353, 5.834f },
    { 3063.00f, 6708.12f, 585.281, 5.834f },
};

static Assault AssaultPosone[]=
{
    { 3975.04f, 5860.19f, 266.310, 0.674f },
    { 3974.83f, 5871.05f, 265.135, 0.674f },
    { 3966.80f, 5874.23f, 265.294, 0.674f },
    { 3969.07f, 5884.81f, 266.298, 0.674f },
    { 3981.83f, 5856.02f, 266.298, 0.674f },
    { 3981.12f, 5845.82f, 266.712, 0.674f },
    { 3989.52f, 5845.91f, 266.793, 0.674f },
    { 3968.80f, 5856.74f, 266.725, 0.674f }
};

static Assault AssaultPostwo[]=
{
    { 4005.12f, 5894.48f, 267.348, 4.926f },
    { 4014.66f, 5880.12f, 267.871, 2.499f },
    { 4011.16f, 5888.21f, 267.826, 3.779f }
};

struct npc_bashir_landingAI : public ScriptedAI
{
    npc_bashir_landingAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

    bool Assault;
    bool CanStart;
    bool Next;

    SummonList summons;
    std::list<uint64> attackers;

    Timer_UnCheked CheckTimer;
    Timer_UnCheked SpawnTimer;
    Timer_UnCheked StartSpawnTimer;
    Timer_UnCheked EndTimer;
    uint8 Wave;
    uint64 AetherGUID;
    uint64 CollectorGUID;

    void Reset() 
    {
        CanStart = true;
        Assault = false;
        Next = true;
        CheckTimer.Reset(3000);
        SpawnTimer.Reset(60000);
        StartSpawnTimer.Reset(240000);
        EndTimer.Reset(1500000);
        attackers.clear();
        Wave = 0;
        AetherGUID = 0;
        CollectorGUID = 0;
        SpawnDefenders();
    }

    void SpawnDefenders()
    {
        for (int i = 0; i < 4; i++)
        {
            switch (i)
            {
                case 0:
                    me->SummonCreature(NPC_AETHER, AssaultPos[i].x, AssaultPos[i].y, AssaultPos[i].z,  AssaultPos[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                    break;
                case 1:
                    me->SummonCreature(NPC_SRANGER, AssaultPos[i].x, AssaultPos[i].y, AssaultPos[i].z,  AssaultPos[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                    break;
                case 2:
                    me->SummonCreature(NPC_SRANGER, AssaultPos[i].x, AssaultPos[i].y, AssaultPos[i].z,  AssaultPos[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                    break;
                case 3:
                    me->SummonCreature(NPC_LIEUTENANT, AssaultPos[i].x, AssaultPos[i].y, AssaultPos[i].z,  AssaultPos[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                    break;
            }
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summons.Summon(summoned);

        if (summoned->GetEntry() == NPC_SRANGER)
            summoned->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USESTANDING);

        if (summoned->GetEntry() == NPC_SUBPRIMAL ||
            summoned->GetEntry() == NPC_CONTROLLER ||
            summoned->GetEntry() == NPC_HARBRINGER ||
            summoned->GetEntry() == NPC_INQUISITOR ||
            summoned->GetEntry() == NPC_RECKONER)
        {
            attackers.push_back(summoned->GetGUID());
            summoned->GetMotionMaster()->MoveFleeing(summoned, 3000);
        }

        if (summoned->GetEntry() == NPC_SSLAVE ||
            summoned->GetEntry() == NPC_FFIEND)
        {
            attackers.push_back(summoned->GetGUID());
            summoned->GetMotionMaster()->MoveFleeing(summoned, 3000);

            if (Creature* Aether = me->GetCreature(AetherGUID))
                summoned->AI()->AttackStart(Aether);
        }

        if (summoned->GetEntry() == NPC_GCOLLECTOR)
        {
            attackers.push_back(summoned->GetGUID());
            CollectorGUID = summoned->GetGUID();
            summoned->SetReactState(REACT_PASSIVE);
        }
        
        if (summoned->GetEntry() == NPC_DISRUPTORT)
        {
            attackers.push_back(summoned->GetGUID());
            if (Creature* Aether = me->GetCreature(AetherGUID))
                Aether->AI()->AttackStart(summoned);
        }

        if (summoned->GetEntry() == NPC_AETHER)
        {
            AetherGUID = summoned->GetGUID();
            DoScriptText(YELL_AETHER_1, summoned);
            summoned->SetReactState(REACT_AGGRESSIVE);
            summoned->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USESTANDING);
        }

        if (summoned->GetEntry() == NPC_ASSISTANT)
            DoScriptText(YELL_ASSISTANT, summoned);

        if (summoned->GetEntry() == NPC_ADEPT)
            DoScriptText(YELL_ADEPT, summoned);

        if (summoned->GetEntry() == NPC_MASTER)
        {
            DoScriptText(YELL_MASTER, summoned);
            if (Creature* Aether = me->GetCreature(AetherGUID))
                DoScriptText(YELL_AETHER_4, Aether);
        }

        if (summoned->GetEntry() == NPC_LIEUTENANT)
        {
            summoned->Mount(21152);
            summoned->SetLevitate(true);
            summoned->SetSpeed(MOVE_FLIGHT, 1.00f);
            summoned->GetMotionMaster()->MovePoint(0, AssaultPos[4].x, AssaultPos[4].y, AssaultPos[4].z);
            summoned->ForcedDespawn(40000);
        }
    }

    void NextWave()
    {
        ++Wave;

        switch (Wave)
        {
            case 1: //1.1
                for (int i = 0; i < 7; i++)
                    me->SummonCreature(NPC_SSLAVE, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                break;
            case 2: //1.2
                for (int i = 0; i < 7; i++)
                {
                    switch (i)
                    {
                        case 0:
                            me->SummonCreature(NPC_FFIEND, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                            me->SummonCreature(NPC_SSLAVE, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                    }
                }
                Next = false;
                break;
            case 3: //2.1
                me->SummonCreature(NPC_ASSISTANT, AssaultPostwo[0].x, AssaultPostwo[0].y, AssaultPostwo[0].z,  AssaultPostwo[0].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);

                for (int i = 0; i < 7; i++)
                    me->SummonCreature(NPC_SUBPRIMAL, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);

                for (int i = 0; i < 7; i++)
                    me->SummonCreature(NPC_SSLAVE, AssaultPosone[i].x+(rand()%6), AssaultPosone[i].y+(rand()%6), AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);

                Next = true;
                break;
            case 4: //2.2
                if (Creature* Aether = me->GetCreature(AetherGUID))
                    DoScriptText(YELL_AETHER_2, Aether);

                for (int i = 0; i < 7; i++)
                {
                    switch (i)
                    {
                        case 0:
                        case 3:
                        case 6:
                            me->SummonCreature(NPC_DISRUPTORT, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 1:
                        case 2:
                        case 4:
                        case 5:
                            me->SummonCreature(NPC_SUBPRIMAL, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                    }
                }
                Next = false;
                break;
            case 5: //3.1
                me->SummonCreature(NPC_ADEPT, AssaultPostwo[1].x, AssaultPostwo[1].y, AssaultPostwo[1].z,  AssaultPostwo[1].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);

                for (int i = 0; i < 7; i++)
                {
                    switch (i)
                    {
                        case 0:
                            me->SummonCreature(NPC_CONTROLLER, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 3:
                        case 6:
                            me->SummonCreature(NPC_HARBRINGER, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 1:
                        case 4:
                            me->SummonCreature(NPC_INQUISITOR, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 2:
                        case 5:
                            me->SummonCreature(NPC_RECKONER, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                    }
                }
                Next = true;
                break;
            case 6: //3.2
                if (Creature* Aether = me->GetCreature(AetherGUID))
                    DoScriptText(YELL_AETHER_3, Aether);

                for (int i = 0; i < 8; i++)
                {
                    switch (i)
                    {
                        case 1:
                        case 4:
                            me->SummonCreature(NPC_INQUISITOR, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 2:
                        case 5:
                            me->SummonCreature(NPC_RECKONER, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 0:
                        case 3:
                        case 6:
                            me->SummonCreature(NPC_HARBRINGER, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                        case 7:
                            me->SummonCreature(NPC_GCOLLECTOR, AssaultPosone[i].x, AssaultPosone[i].y, AssaultPosone[i].z,  AssaultPosone[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                            break;
                    }
                }
                break;
            case 7:
                if (Creature* Aether = me->GetCreature(AetherGUID))
                {
                    if (Creature* Collector = me->GetCreature(CollectorGUID))
                    {
                        Collector->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        Collector->SetReactState(REACT_AGGRESSIVE);
                        Collector->AI()->AttackStart(Aether);
                    }
                }
                Next = false;
                break;
            case 8:
                me->SummonCreature(NPC_MASTER, AssaultPostwo[2].x, AssaultPostwo[2].y, AssaultPostwo[2].z,  AssaultPostwo[2].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500000);
                Assault = true;
                break;
        }
    }

    void EventFail()
    {
        summons.DespawnAll();
        attackers.clear();
        Assault = true;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Assault)
        {
            if (CheckTimer.Expired(diff))
            {
                Map* tmpMap = me->GetMap();

                if (!tmpMap)
                    return;

                if (!attackers.empty())
                {
                    bool alive = false;
                    for (std::list<uint64>::iterator itr = attackers.begin(); itr != attackers.end(); ++itr)
                    {
                        if (Creature* attacker = tmpMap->GetCreature((*itr)))
                        {
                            if (attacker->isAlive())
                            {
                                alive = true;
                                break;
                            }
                        }
                    }

                    if (!alive)
                    {
                        NextWave();
                        SpawnTimer = 180000;
                    }
                 }

                 CheckTimer = 3000;

                 if (Creature* Aether = me->GetCreature(AetherGUID))
                 {
                     if (Aether->isAlive())
                         return;
                     else EventFail();
                 }
            }


            if (Next)
            {
                if (SpawnTimer.Expired(diff))
                {
                    NextWave();
                    SpawnTimer = 180000;
                }
            }
        }

        if (CanStart)
        {
            if (StartSpawnTimer.Expired(diff))
            {
                NextWave();
                CanStart = false;
            }
        }

        if (EndTimer.Expired(diff))
        {
            EventFail();
        }
    }
};

CreatureAI* GetAI_npc_bashir_landing(Creature* creature)
{
    return new npc_bashir_landingAI (creature);
}

/*######
## npc_banishing_crystal
######*/

enum
{
    SPELL_MASTER        = 40828,
    SPELL_BANISHMENT    = 40857,

    NPC_BCREDIT         = 23327
};

struct npc_banishing_crystalAI : public ScriptedAI
{
    npc_banishing_crystalAI(Creature* creature) : ScriptedAI(creature) {}

    uint64 PlayerGUID;

    void Reset() 
    {
        PlayerGUID = 0;
        DoCast(me, SPELL_BANISHMENT);
        GetPlayer();
		me->SetReactState(REACT_PASSIVE);
    }

    void GetPlayer()
    { 
        Map* map = me->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();

        for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (Player* player = itr->getSource())
            {
                if (me->IsWithinDistInMap(player, 30.0f) && (player->GetQuestStatus(11026) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11051) == QUEST_STATUS_INCOMPLETE))
                    PlayerGUID = player->GetGUID();
            }
        }
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_MASTER)
        {
            if (Player* player = me->GetPlayerInWorld(PlayerGUID))
                player->KilledMonster(NPC_BCREDIT, me->GetGUID());
        }
    }
};

CreatureAI* GetAI_npc_banishing_crystal(Creature* creature)
{
    return new npc_banishing_crystalAI (creature);
}

/*####
# npc_oscillating_frequency_scanner_master_bunny
####*/
enum ScannerMasterBunny
{
    NPC_OSCILLATING_FREQUENCY_SCANNER_TOP_BUNNY = 21759,
    SPELL_OSCILLATION_FIELD                     = 37408,
    QUEST_GAUGING_THE_RESONANT_FREQUENCY        = 10594
};

struct npc_oscillating_frequency_scanner_master_bunnyAI : public ScriptedAI
{
    npc_oscillating_frequency_scanner_master_bunnyAI(Creature* creature) : ScriptedAI(creature) { }

    uint64 playerGuid;
    uint32 timer;

    void Reset()
    {
        if (GetClosestCreatureWithEntry(me, NPC_OSCILLATING_FREQUENCY_SCANNER_TOP_BUNNY, 25))
            me->ForcedDespawn();
        else
        {
            me->SummonCreature(21759, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 0.5f, me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 50000);
            if (urand(0, 1) == 1)
                me->SummonCreature(21798, me->GetPositionX() + 5.0f, me->GetPositionY(), me->GetPositionZ() + 0.5f, me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1000);
            else 
                me->SummonCreature(21798, me->GetPositionX(), me->GetPositionY() + 10.0f, me->GetPositionZ() + 0.5f, me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000);
            me->SummonGameObject(184926, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), 0, 0, 0, 0, 50000);
            me->ForcedDespawn(50000);
        }

        timer = 500;
    }

    void IsSummonedBy(Unit* summoner)
    {
        if (summoner->GetTypeId() == TYPEID_PLAYER)
            playerGuid = summoner->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
        if (timer <= diff)
        {
            if (Player* player = Unit::GetPlayerInWorld(playerGuid))
            {
                if (player->GetQuestStatus(10594) == QUEST_STATUS_INCOMPLETE)
                {
                    if(player->HasAura(37408, 0))
                    {
                        uint32 count = 0;
                        for (Unit::AuraMap::const_iterator itr = player->GetAuras().begin(); itr != player->GetAuras().end(); ++itr)
                        {
                            if (itr->second->GetId() == 37408)
                                ++count;
                        }
                        if(count >= 4)
                            player->AreaExploredOrEventHappens(10594);
                        else
                            me->AddAura(SPELL_OSCILLATION_FIELD, player);
                    }
                    else
                        me->AddAura(SPELL_OSCILLATION_FIELD, player);
                }
            }
            timer = 3000;
        }
        else
            timer -= diff;
    }
};

CreatureAI* GetAI_npc_oscillating_frequency_scanner_master_bunny(Creature* creature)
{
    return new npc_oscillating_frequency_scanner_master_bunnyAI(creature);
}

/*####
# npc_scalewing_serpent (20749)
####*/

struct npc_scalewing_serpentAI : public ScriptedAI
{
    npc_scalewing_serpentAI(Creature* creature) : ScriptedAI(creature) { }

    uint32 timer;

    void Reset()
    {
        timer = 3000;
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {
        if(target->GetTypeId() == TYPEID_PLAYER)
            if(spell->Id == 37841)
                if(((Player*)target)->GetQuestStatus(10594) == QUEST_STATUS_INCOMPLETE)
                    if(target->HasAura(37830, 0))
                        ((Player*)target)->KilledMonster(21910, 0);
    }
    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (timer <= diff)
        {
            DoCast(me->GetVictim(), 37841);
            timer = 3000;
        }
        else
            timer -= diff;
    }
};

CreatureAI* GetAI_npc_scalewing_serpent(Creature* creature)
{
    return new npc_scalewing_serpentAI(creature);
}

/*############################
# EVENT: Shartuul's Transporter
# WowWiki: http://www.wowwiki.com/Shartuul%27s_Transporter_(event)
# ToDo: 1. Add flames on shield destroying
#       2. Spirit channeling on charm
#       3. Fix spells for 2nd boss(40659 dive-bomb, fel flames)
#       4. Fix state of 2nd boss in Phase 1.
#       5. Fix Phase 3
#       6. Fix reset of event
#       7. Remove world state on entering phase 2
############################*/

enum ShartuulEvent
{
    NPC_LR_E_INVIS_MAN          = 23059,
    NPC_LR_E_INFERNAL           = 23075,
    NPC_LR_E_INVIS_MAN_LG       = 23260,
    NPC_LR_SHIELD_ZAPPER        = 23500,
    NPC_FELGUARD_DEGRADER       = 23055,
    NPC_DOOMGUARD_PUNISHER      = 23113,
    NPC_FELHOUND_DEFENDER       = 23173,
    NPC_FEL_IMP_DEFENDER        = 23078,
    NPC_GANARG_UNDERLING        = 23199,
    NPC_MOARG_TORMENTER         = 23212,
    NPC_SHIVAN_ASSASIN          = 23220,

    SPELL_LR_GREEN_LIGHTNING    = 40146,
    SPELL_LR_GREEN_MATTER       = 40071,
    SPELL_LR_SPAWN_LIGHTNING    = 39979,
    SPELL_VISUAL_SHIELD         = 40158,
    SPELL_SMASH_HIT             = 40222,
    // Seconds Demon spells.
    SPELL_SHADOWFORM            = 40973,
    SPELL_PUNISHING_BLOW        = 40560,
    SPELL_FEL_FLAMES            = 40561,
    SPELL_THROW_AXE             = 40563,
    SPELL_CONSUME_ESSENCE       = 40565,
    SPELL_SUPER_JUMP            = 40493,
    SPELL_DIVE_BOMB             = 35181,
    // Phase 2
    SPELL_LR_CHARM_2            = 40382,
    // Phase 3
    SPELL_LR_CHARM_3            = 40523,
    
    
    WORLD_STATE_SHIELD          = 3055,
    WORLD_STATE_SHIELD_1        = 3054
};

struct PositionL
{
    float x, y, z, o;
};

static PositionL LightPosition[]=
{
    {2690.24, 7117.25, 364.938, 0.939785},
    {2704.13, 7162.75, 364.691, 0.943048},
    {2715.53, 7144.8,  365.37,  1.16982},
    {2725.9,  7163.4,  364.607, 0.916224},
    {2741.08, 7151.16, 365.06,  0.924075},
    {2741.92, 7130.31, 365.924, 0.990747},
    {2765.09, 7133.96, 366.076, 1.12112},
    {2768.67, 7112.34, 365.399, 0.876948},
    {2755.04, 7088.44, 365.715, 0.982733},
    {2741.41, 7065.12, 365.476, 1.05342},
    {2736.47, 7083.63, 365.849, 1.07698},
    {2751.33, 7111.23, 364.748, 1.07698},
    {2719.69, 7076.05, 364.713, 1.26083},
    {2718.54, 7091.67, 364.808, 1.12567},
    {2700.42, 7101.23, 365.549, 1.19871},
    {2691.34, 7085.07, 364.519, 1.07305},
    {2674.62, 7094.84, 363.987, 1.02944},
    {2671.61, 7117.3,  364.376, 0.907705},
    {2687.39, 7139.68, 364.079, 0.943048}
};

/*###########
# npc_23260
# Description: this one cast Green Lightnings in NPC 23059 untill event started.
###########*/

struct npc_23260AI : public ScriptedAI
{
    npc_23260AI(Creature* creature) : ScriptedAI(creature) { }

    uint32 timer;

    void Reset()
    {
        timer = urand(1000, 6000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(timer)
        {
            if (timer <= diff)
            {
                if(Unit* target = FindCreature(NPC_LR_E_INVIS_MAN, 20.0f, me))
                    DoCast(target, SPELL_LR_GREEN_LIGHTNING, true);
                timer = urand(7000, 24000);
            }
            else
                timer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_23260(Creature* creature)
{
    return new npc_23260AI(creature);
}

/*###########
# npc_23059
# Description: This one gets lights from 23260 and on EventStart cast beam in nearest 23059
###########*/

struct npc_23059AI : public ScriptedAI
{
    npc_23059AI(Creature* creature) : ScriptedAI(creature) { }

    uint32 timer;

    void Reset()
    {
    }
    
    void EventStarted(bool Started)
    {
        if(Started)
        {
            std::list<Creature*> npc_23059List = FindAllCreaturesWithEntry(NPC_LR_E_INVIS_MAN, 200);
            uint32 myDbTableGUid = me->GetDBTableGUIDLow();
            if (myDbTableGUid >= 1010500 && myDbTableGUid <= 1010503)
            {
                for (std::list<Creature *>::iterator i = npc_23059List.begin(); i != npc_23059List.end(); i++)
                {
                    uint32 targetDbTableGuid = (*i)->GetDBTableGUIDLow();
                    if ((targetDbTableGuid == myDbTableGUid + 1) || (targetDbTableGuid == myDbTableGUid - 3))
                        me->CastSpell(((Unit*)(*i)), SPELL_LR_GREEN_MATTER, true);
                }
            }

            /*if(me->GetDBTableGUIDLow() == 1010500)
            {
                for(std::list<Creature *>::iterator i = npc_23059List.begin(); i != npc_23059List.end(); i++)
                {
                    if((*i)->GetDBTableGUIDLow() == 1010501)
                        me->CastSpell(((Unit*)(*i)), SPELL_LR_GREEN_MATTER, true);
                }
            }
            else if(me->GetDBTableGUIDLow() == 1010501)
            {
                for(std::list<Creature *>::iterator i = npc_23059List.begin(); i != npc_23059List.end(); i++)
                {
                    if((*i)->GetDBTableGUIDLow() == 1010502)
                        me->CastSpell(((Unit*)(*i)), SPELL_LR_GREEN_MATTER, true);
                }
            }
            else if(me->GetDBTableGUIDLow() == 1010502)
            {
                for(std::list<Creature *>::iterator i = npc_23059List.begin(); i != npc_23059List.end(); i++)
                {
                    if((*i)->GetDBTableGUIDLow() == 1010503)
                        me->CastSpell(((Unit*)(*i)), SPELL_LR_GREEN_MATTER, true);
                }
            }
            else if(me->GetDBTableGUIDLow() == 1010503)
            {
                for(std::list<Creature *>::iterator i = npc_23059List.begin(); i != npc_23059List.end(); i++)
                {
                    if((*i)->GetDBTableGUIDLow() == 1010500)
                        me->CastSpell(((Unit*)(*i)), SPELL_LR_GREEN_MATTER, true);
                }
            }*/
        }
    }
};

CreatureAI* GetAI_npc_23059(Creature* creature)
{
    return new npc_23059AI(creature);
}

/*###########
# npc_23075
# Description: Phase 1: cast Light and after summon 2 types of creatures
#              Phase 2: cast Light and summon 1 type of creatures
###########*/

struct npc_23075AI : public ScriptedAI
{
    npc_23075AI(Creature* creature) : ScriptedAI(creature), Summons(me) { }

    SummonList Summons;
    uint32 timer;
    uint32 SummonTimer;
    uint8 Phase;
    uint32 SummonCounter;
    uint32 DeadSummonCounter;
    bool Felhound;
    bool FelImp;
    bool CanSummon;
    bool GanargUnderling;
    bool Moarg;
    bool ShivanAssasin;

    void Reset()
    {
        timer = 1000;
        SummonTimer = 3000;
        Phase = 0;
        SummonCounter = 0;
        DeadSummonCounter = 0;
        Felhound = false;
        FelImp = false;
        CanSummon = false;
        GanargUnderling = false;
        Moarg = false;
        ShivanAssasin = false;
    }

    void JustSummoned(Creature *summoned)
    {
        if(summoned->GetEntry() == NPC_FELHOUND_DEFENDER || summoned->GetEntry() == NPC_FEL_IMP_DEFENDER)
        {
            if(Unit* target = FindCreature(NPC_FELGUARD_DEGRADER, 300, me))
            {
                summoned->AI()->AttackStart(target);
                summoned->AddThreat(target, 100000.0f);
                me->DisappearAndDie();
            }
        }
        else if(summoned->GetEntry() == NPC_GANARG_UNDERLING || summoned->GetEntry() == NPC_MOARG_TORMENTER || summoned->GetEntry() == NPC_SHIVAN_ASSASIN)
        {
            if(Unit* target = FindCreature(NPC_DOOMGUARD_PUNISHER, 300, me))
            {
                summoned->AI()->AttackStart(target);
                summoned->AddThreat(target, 100000.0f);
                me->DisappearAndDie();
            }
        }
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {

            if(timer)
            {
                if (timer <= diff)
                {
                    if(Unit* target = FindCreature(NPC_LR_SHIELD_ZAPPER, 200.0f, me))
                        DoCast(target, SPELL_LR_SPAWN_LIGHTNING, true);
                    timer = 0;
                }
                else
                    timer -= diff;
            }

            if(SummonTimer < diff)
            {
                if(Felhound)
                    me->SummonCreature(NPC_FELHOUND_DEFENDER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                else if(FelImp)
                    me->SummonCreature(NPC_FEL_IMP_DEFENDER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                else if(GanargUnderling)
                    me->SummonCreature(NPC_GANARG_UNDERLING, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                else if(Moarg)
                    me->SummonCreature(NPC_MOARG_TORMENTER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
                else if(ShivanAssasin)
                    me->SummonCreature(NPC_SHIVAN_ASSASIN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
                SummonTimer = 0;
            } else SummonTimer -= diff;
    }
};

CreatureAI* GetAI_npc_23075(Creature* creature)
{
    return new npc_23075AI(creature);
}

/*###########
# npc_23173
# Description: Felhound Defender
###########*/

struct npc_23173AI : public ScriptedAI
{
    npc_23173AI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
        me->SetAggroRange(150.0f);
        me->SetIsDistanceToHomeEvadable(false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23173(Creature* creature)
{
    return new npc_23173AI(creature);
}

/*###########
# npc_23078
# Description: Fel Imp Defender
###########*/

struct npc_23078AI : public ScriptedAI
{
    npc_23078AI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
        me->SetAggroRange(150.0f);
        me->SetIsDistanceToHomeEvadable(false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23078(Creature* creature)
{
    return new npc_23078AI(creature);
}

/*###########
# npc_23220
# Description: 3rd controlled demon. 3 phased.
###########*/

struct npc_23220AI : public ScriptedAI
{
    npc_23220AI(Creature* creature) : ScriptedAI(creature) { }

    uint32 CheckTimer;
    uint64 PlayerGUID;

    void Reset()
    {
        CheckTimer = 3000;
        me->setFaction(14);
        me->SetAggroRange(150.0f);
        me->SetIsDistanceToHomeEvadable(false);
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
            if(Creature* npc_23500 = GetClosestCreatureWithEntry(me, NPC_LR_SHIELD_ZAPPER, 150))
                npc_23500->AI()->EventPulse(me, 2);
            me->SetSpeed(MOVE_WALK, 1.5, true);
            me->SetSpeed(MOVE_RUN, 1.5, true);
        }
    }
    
    void JustDied(Unit *killer)
    {
        if(killer->GetTypeId() == TYPEID_UNIT)
        {
            me->Respawn();
            me->Kill(killer);
            if (Player* plr = me->GetPlayerInWorld(PlayerGUID))
                plr->CastSpell(plr, SPELL_LR_CHARM_3, true);
        }
        else
            killer->CastSpell(killer, SPELL_LR_CHARM_3, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->isCharmed() && me->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            // remove charm when not in Legion Ring
            if(CheckTimer < diff)
            {
                Unit* charmer = me->GetCharmerOrOwnerPlayerOrPlayerItself();
                if(charmer->GetAreaId() != 4008)
                    me->RemoveCharmedOrPossessedBy(charmer);
                CheckTimer = 2000;

            }
            else
                CheckTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23220(Creature* creature)
{
    return new npc_23220AI(creature);
}

/*###########
# npc_23113
# Description: 2nd controlled demon. Stays with not selectable flag and banished untill 1st shield isn't broken.
###########*/

struct npc_23113AI : public ScriptedAI
{
    npc_23113AI(Creature* creature) : ScriptedAI(creature) { }

    uint32 PunishingBlowTimer;
    uint32 FelFlamesTimer;
    uint32 CheckTimer;
    uint64 PlayerGUID;

    void Reset()
    {
        PunishingBlowTimer = 5000;
        FelFlamesTimer = urand(15000, 25000);
        CheckTimer = 3000;
       /* me->CastSpell(me, SPELL_SHADOWFORM, true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);*/
        me->SetAggroRange(150.0f);
        me->SetIsDistanceToHomeEvadable(false);
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
            if(Creature* npc_23500 = GetClosestCreatureWithEntry(me, NPC_LR_SHIELD_ZAPPER, 150))
                npc_23500->AI()->EventPulse(me, 1);
            me->SetSpeed(MOVE_WALK, 1.5, true);
            me->SetSpeed(MOVE_RUN, 1.5, true);
        }
    }
    
    void EventStarted(bool Started, Unit* Hitter)
    {
        if(Started)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->RemoveAurasDueToSpell(SPELL_SHADOWFORM);
            me->AI()->AttackStart(Hitter);
            me->setFaction(14);
        }
    }
    
    void JustDied(Unit *killer)
    {
        if(killer->GetTypeId() == TYPEID_UNIT)
        {
            me->Respawn();
            me->Kill(killer);
            if (Player* plr = me->GetPlayerInWorld(PlayerGUID))
                plr->CastSpell(plr, SPELL_LR_CHARM_2, true);
        }
        else
            killer->CastSpell(killer, SPELL_LR_CHARM_2, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->isCharmed() && me->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            // remove charm when not in Legion Ring
            if(CheckTimer < diff)
            {
                Unit* charmer = me->GetCharmerOrOwnerPlayerOrPlayerItself();
                if(charmer->GetAreaId() != 4008)
                    me->RemoveCharmedOrPossessedBy(charmer);
                CheckTimer = 2000;

            }
            else
                CheckTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        if(PunishingBlowTimer < diff)
        {
            me->CastSpell(me->GetVictim(), SPELL_PUNISHING_BLOW, true);
            PunishingBlowTimer = 10000;
        } else PunishingBlowTimer -= diff;

        if(FelFlamesTimer < diff)
        {
            me->CastSpell(me->GetVictim(), SPELL_FEL_FLAMES, true);
            FelFlamesTimer = urand(15000,25000);
        } else FelFlamesTimer -= diff;
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23113(Creature* creature)
{
    return new npc_23113AI(creature);
}

/*###########
# npc_23500
# Description: this one is controller of the whole event.
###########*/

struct npc_23500AI : public ScriptedAI
{
    npc_23500AI(Creature* creature) : ScriptedAI(creature) { }

    uint32 ShieldPercent;
    uint32 SummonLightFelhoundTimer;
    uint32 SummonLightFelimpTimer;
    uint32 SummonGanargUnderling;
    uint32 Counter;
    bool phase2;
    bool phase3;
    bool checked;
    bool CanSummonLightFelhoundTimer;
    bool CanSummonLightFelimpTimer;
    bool CanSummonGanargUnderling;
    bool Counted0;
    bool Counted1;
    bool Counted2;
    bool Counted3;
    bool Counted4;
    bool Counted5;
    bool Counted6;
    bool Counted7;

    void Reset()
    {
        me->CastSpell(me, SPELL_VISUAL_SHIELD, true);
        ShieldPercent = 100;
        SummonLightFelhoundTimer = 0;
        SummonLightFelimpTimer = 0;
        SummonGanargUnderling = 0;
        Counter = 0;
        phase2 = false;
        phase3 = false;
        checked = false;
        CanSummonLightFelhoundTimer = false;
        CanSummonLightFelimpTimer = false;
        CanSummonGanargUnderling = false;
        Counted0 = false;
        Counted1 = false;
        Counted2 = false;
        Counted3 = false;
        Counted4 = false;
        Counted5 = false;
        Counted6 = false;
        Counted7 = false;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (damage >= me->GetHealth() && done_by != me)
            damage = 0;
    }

    void SpellHit(Unit *Hitter, const SpellEntry *spell)
    {
        if(Hitter->GetTypeId() == TYPEID_UNIT)
        {
            if(spell->Id == SPELL_SMASH_HIT)
            {
                if(Player* plr = Hitter->GetCharmerOrOwnerPlayerOrPlayerItself())
                {
                    ShieldPercent = (ShieldPercent - 12.5);
                    if(ShieldPercent <= 50 && !checked)
                    {
                        checked = true;
                    }
                    if(ShieldPercent <= 12)
                    {
                        ShieldPercent = 0;
                        if(Unit* doomguard = FindCreature(NPC_DOOMGUARD_PUNISHER, 20, me))
                        {
                            CAST_AI(npc_23113AI, ((Creature*)doomguard)->AI())->EventStarted(true, Hitter);
                            CAST_AI(npc_23113AI, ((Creature*)doomguard)->AI())->PlayerGUID = plr->GetGUID();
                        }
                        CanSummonLightFelhoundTimer = false;
                        CanSummonLightFelimpTimer = false;
                        SummonLightFelhoundTimer = 0;
                        SummonLightFelimpTimer = 0;
                        me->RemoveAurasDueToSpell(SPELL_VISUAL_SHIELD);
                        // ToDo: add fires over portal
                    }
                    plr->SendUpdateWorldState(WORLD_STATE_SHIELD_1, ShieldPercent);
                }
            }
        }
    }

    void EventStarted(bool Started, bool StartedPhase2, bool StartedPhase3, Player* plr)
    {
        if(Started)
        {
            std::list<Creature*> npc_23260List = FindAllCreaturesWithEntry(NPC_LR_E_INVIS_MAN_LG, 200);
            if (!npc_23260List.empty())
            {
                for(std::list<Creature *>::iterator i = npc_23260List.begin(); i != npc_23260List.end(); i++)
                    CAST_AI(npc_23260AI, (*i)->AI())->timer = 0;
            }
            std::list<Creature*> npc_23059List = FindAllCreaturesWithEntry(NPC_LR_E_INVIS_MAN, 200);
            if (!npc_23059List.empty())
            {
                for(std::list<Creature *>::iterator i = npc_23059List.begin(); i != npc_23059List.end(); i++)
                {
                    (*i)->GetMotionMaster()->MovePoint(1, (*i)->GetPositionX(), (*i)->GetPositionY(), (*i)->GetPositionZ()-7);
                    CAST_AI(npc_23059AI, (*i)->AI())->EventStarted(true);
                }
            }
            CanSummonLightFelhoundTimer = true;
            CanSummonLightFelimpTimer = true;
            SummonLightFelhoundTimer = 1000;
            SummonLightFelimpTimer = 2000;
            if(plr)
            {
                plr->SendUpdateWorldState(WORLD_STATE_SHIELD, 1);
                plr->SendUpdateWorldState(WORLD_STATE_SHIELD_1, ShieldPercent);
            }
        }
        else if(StartedPhase2)
        {
            phase2 = true;
            CanSummonGanargUnderling = true;
            SummonGanargUnderling = 1000;
            if(plr)
                plr->SendUpdateWorldState(WORLD_STATE_SHIELD, 0);
        }
        else if(StartedPhase3)
        {
            phase3 = true;
        }
    }
    
    void EventPulse(Unit* pSender, uint32 PulseEventNumber)
    {
        if(PulseEventNumber == 1)
            EventStarted(false, true, false, me->GetCharmerOrOwnerPlayerOrPlayerItself());
        else if(PulseEventNumber == 2)
            EventStarted(false, false, true, me->GetCharmerOrOwnerPlayerOrPlayerItself());
    }

    void UpdateAI(const uint32 diff)
    {
        if(!phase2)
        {
            if(CanSummonLightFelhoundTimer) // Spawn 2 hounds every 6 sec in random position.
            {
                if(SummonLightFelhoundTimer < diff)
                {
                    uint32 PartOne = urand(0,9);
                    uint32 PartTwo = urand(10,18);
                    if(Creature* Felhound1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[PartOne].x, LightPosition[PartOne].y, LightPosition[PartOne].z, LightPosition[PartOne].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        CAST_AI(npc_23075AI, Felhound1->AI())->Felhound = true;
                    if(Creature* Felhound2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[PartTwo].x, LightPosition[PartTwo].y, LightPosition[PartTwo].z, LightPosition[PartTwo].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        CAST_AI(npc_23075AI, Felhound2->AI())->Felhound = true;
                    SummonLightFelhoundTimer = 5000;
                } else SummonLightFelhoundTimer -= diff;
            }

            if(CanSummonLightFelimpTimer) // If shield >50% - spawn 1 Imp, else 2. 
            {
                if(SummonLightFelimpTimer < diff)
                {
                    uint32 PartOne = urand(0,9);
                    uint32 PartTwo = urand(10,18);
                    uint32 PartThree = urand(0,18);
                    if(!checked)
                    {
                        if(Creature* Imp = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[PartThree].x, LightPosition[PartThree].y, LightPosition[PartThree].z, LightPosition[PartThree].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Imp->AI())->FelImp = true;
                    }
                    else
                    {
                        if(Creature* Imp1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[PartOne].x, LightPosition[PartOne].y, LightPosition[PartOne].z, LightPosition[PartOne].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Imp1->AI())->FelImp = true;
                        if(Creature* Imp2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[PartTwo].x, LightPosition[PartTwo].y, LightPosition[PartTwo].z, LightPosition[PartTwo].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Imp2->AI())->FelImp = true;
                    }
                    SummonLightFelimpTimer = 6000;
                } else SummonLightFelimpTimer -= diff;
            }
        }
        else if(phase2)
        {
            if(CanSummonGanargUnderling)
            {
                if(SummonGanargUnderling < diff)
                {
                    if(Counter == 0 && !Counted0)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[1].x, LightPosition[1].y, LightPosition[1].z, LightPosition[1].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[18].x, LightPosition[18].y, LightPosition[18].z, LightPosition[18].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        Counted0 = true;
                    }
                    if(Counter == 2 && !Counted1)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[1].x, LightPosition[1].y, LightPosition[1].z, LightPosition[1].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[18].x, LightPosition[18].y, LightPosition[18].z, LightPosition[18].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        if(Creature* Ganarg3 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[13].x, LightPosition[13].y, LightPosition[13].z, LightPosition[13].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg3->AI())->GanargUnderling = true;
                        Counted1 = true;
                    }
                    if(Counter == 5 && !Counted2)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[1].x, LightPosition[1].y, LightPosition[1].z, LightPosition[1].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[18].x, LightPosition[18].y, LightPosition[18].z, LightPosition[18].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        if(Creature* Ganarg3 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[13].x, LightPosition[13].y, LightPosition[13].z, LightPosition[13].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg3->AI())->GanargUnderling = true;
                        Counted2 = true;
                    }
                    if(Counter == 8 && !Counted3)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[16].x, LightPosition[16].y, LightPosition[16].z, LightPosition[16].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[15].x, LightPosition[15].y, LightPosition[15].z, LightPosition[15].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        if(Creature* Ganarg3 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[13].x, LightPosition[13].y, LightPosition[13].z, LightPosition[13].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg3->AI())->GanargUnderling = true;
                        if(Creature* Ganarg6 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[1].x, LightPosition[1].y, LightPosition[1].z, LightPosition[1].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg6->AI())->GanargUnderling = true;
                        if(Creature* Moarg = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[14].x, LightPosition[14].y, LightPosition[14].z, LightPosition[14].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Moarg->AI())->Moarg = true;
                        if(Creature* Ganarg4 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[4].x, LightPosition[4].y, LightPosition[4].z, LightPosition[4].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg4->AI())->GanargUnderling = true;
                        if(Creature* Ganarg5 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[5].x, LightPosition[5].y, LightPosition[5].z, LightPosition[5].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg5->AI())->GanargUnderling = true;
                        Counted3 = true;
                    }
                    if(Counter == 14 && !Counted4)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[14].x, LightPosition[14].y, LightPosition[14].z, LightPosition[14].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[15].x, LightPosition[15].y, LightPosition[15].z, LightPosition[15].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        if(Creature* Ganarg3 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[13].x, LightPosition[13].y, LightPosition[13].z, LightPosition[13].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg3->AI())->GanargUnderling = true;
                        if(Creature* Ganarg6 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[1].x, LightPosition[1].y, LightPosition[1].z, LightPosition[1].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg6->AI())->GanargUnderling = true;
                        if(Creature* Moarg = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[16].x, LightPosition[16].y, LightPosition[16].z, LightPosition[16].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Moarg->AI())->Moarg = true;
                        if(Creature* Ganarg4 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[4].x, LightPosition[4].y, LightPosition[4].z, LightPosition[4].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg4->AI())->GanargUnderling = true;
                        if(Creature* Ganarg5 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[5].x, LightPosition[5].y, LightPosition[5].z, LightPosition[5].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg5->AI())->GanargUnderling = true;
                        Counted4 = true;
                    }
                    if(Counter == 20 && !Counted5)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[16].x, LightPosition[16].y, LightPosition[16].z, LightPosition[16].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[15].x, LightPosition[15].y, LightPosition[15].z, LightPosition[15].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        if(Creature* Ganarg3 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[13].x, LightPosition[13].y, LightPosition[13].z, LightPosition[13].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg3->AI())->GanargUnderling = true;
                        if(Creature* Ganarg6 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[1].x, LightPosition[1].y, LightPosition[1].z, LightPosition[1].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg6->AI())->GanargUnderling = true;
                        if(Creature* Moarg = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[12].x, LightPosition[12].y, LightPosition[12].z, LightPosition[12].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Moarg->AI())->Moarg = true;
                        if(Creature* Ganarg4 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[4].x, LightPosition[4].y, LightPosition[4].z, LightPosition[4].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg4->AI())->GanargUnderling = true;
                        if(Creature* Ganarg5 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[5].x, LightPosition[5].y, LightPosition[5].z, LightPosition[5].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg5->AI())->GanargUnderling = true;
                        Counted5 = true;
                    }
                    if(Counter == 26 && !Counted6)
                    {
                        if(Creature* Ganarg1 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[16].x, LightPosition[16].y, LightPosition[16].z, LightPosition[16].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg1->AI())->GanargUnderling = true;
                        if(Creature* Ganarg2 = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[15].x, LightPosition[15].y, LightPosition[15].z, LightPosition[15].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, Ganarg2->AI())->GanargUnderling = true;
                        Counted6 = true;
                    }
                    if(Counter == 28 && !Counted7)
                    {
                        if(Creature* ShivanAssasin = me->SummonCreature(NPC_LR_E_INFERNAL, LightPosition[16].x, LightPosition[16].y, LightPosition[16].z, LightPosition[16].o, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            CAST_AI(npc_23075AI, ShivanAssasin->AI())->ShivanAssasin = true;
                        Counted7 = true;
                    }
                    SummonGanargUnderling = 6000;
                } else SummonGanargUnderling -= diff;
            }
        }
        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_23500(Creature* creature)
{
    return new npc_23500AI(creature);
}

/*###########
# npc_23055
# Description: 1st controlled demon. Starts whole event.
###########*/

struct npc_23055AI : public ScriptedAI
{
    npc_23055AI(Creature* creature) : ScriptedAI(creature) { }

    uint32 CheckTimer;

    void Reset()
    {
        CheckTimer = 3000;
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
            // disable event start, untill completed
            /*std::list<Creature*> npc_23500List = FindAllCreaturesWithEntry(NPC_LR_SHIELD_ZAPPER, 150);
            if (!npc_23500List.empty())
            {
                for(std::list<Creature *>::iterator i = npc_23500List.begin(); i != npc_23500List.end(); i++)
                    CAST_AI(npc_23500AI, (*i)->AI())->EventStarted(true, false, false, me->GetCharmerOrOwnerPlayerOrPlayerItself());
            }*/
            me->SetSpeed(MOVE_WALK, 1.5, true);
            me->SetSpeed(MOVE_RUN, 1.5, true);
        }
    }

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == 40309)
		{
			caster->CastSpell(me, 39985, true);
		}
	}

    void UpdateAI(const uint32 diff)
    {
        if(me->isCharmed() && me->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            // remove charm when not in Legion Ring
            if(CheckTimer < diff)
            {
                Unit* charmer = me->GetCharmerOrOwnerPlayerOrPlayerItself();
                if(charmer->GetAreaId() != 4008)
                    me->RemoveCharmedOrPossessedBy(charmer);
                CheckTimer = 2000;

            }
            else
                CheckTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23055(Creature* creature)
{
    return new npc_23055AI(creature);
}

/*###########
# npc_23199
# Description: Ganarg Underling
###########*/

struct npc_23199AI : public ScriptedAI
{
    npc_23199AI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
        me->SetAggroRange(150.0f);
        me->SetIsDistanceToHomeEvadable(false);
    }

    void JustDied(Unit* who)
    {
        if(Creature* npc_23500 = GetClosestCreatureWithEntry(me, NPC_LR_SHIELD_ZAPPER, 150))
            CAST_AI(npc_23500AI, npc_23500->AI())->Counter++;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23199(Creature* creature)
{
    return new npc_23199AI(creature);
}

struct npc_20216AI : public ScriptedAI
{
    npc_20216AI(Creature* creature) : ScriptedAI(creature) { }

    Timer BurningRageTimer;
    Timer GreviousWoundsTimer;

    void Reset()
    {
        me->SetIsDistanceToHomeEvadable(false);
        BurningRageTimer.Reset(10000);
        GreviousWoundsTimer.Reset(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(BurningRageTimer.Expired(diff))
        {
            me->CastSpell(me, 38771, false);
            DoTextEmote(-1200469, 0, false);
            BurningRageTimer = 30000;
        }

        if(GreviousWoundsTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 38772, false);
            GreviousWoundsTimer = 20000;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20216(Creature* creature)
{
    return new npc_20216AI(creature);
}

struct npc_22114AI : public ScriptedAI
{
    npc_22114AI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
        me->GetMotionMaster()->MovePoint(1000, 2705.71, 5608.55, -10.8015);
        me->SetIsDistanceToHomeEvadable(false);
        me->SetAggroRange(90.0);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        switch(PointId)
        {
            case 1000:
                me->GetMotionMaster()->MovePoint(2000, 2699.25, 5574.15, -10.1521);
                break;
            case 2000:
                me->GetMotionMaster()->MovePoint(3000, 2697.14, 5559.02, -5.20213);
                break;
            case 3000:
                me->GetMotionMaster()->MovePoint(4000, 2688.49, 5548.13, -3.53532);
                if(Unit* target = FindCreature(20216, 30.0f, me))
                    ((Creature*)target)->AI()->AttackStart(me);
                break;
            case 4000:
                me->GetMotionMaster()->MovePoint(5000, 2672.79, 5550.62, -4.39258);
                break;
            case 5000:
                me->GetMotionMaster()->MovePoint(6000, 2669.28, 5562.24, -6.88615);
                break;
            case 6000:
                me->GetMotionMaster()->MovePoint(7000, 2675.93, 5579.65, -10.6906);
                break;
            case 7000:
                me->GetMotionMaster()->MovePoint(8000, 2678.15, 5597.98, -11.1513);
                break;
            case 8000:
                me->GetMotionMaster()->MovePoint(9000, 2662.81, 5622.31, -11.8684);
                break;
            case 9000:
                me->GetMotionMaster()->MovePoint(10000, 2672.65, 5647.28, -13.2614);
                break;
            case 10000:
                 me->ForcedDespawn(3000);
                break;
        }
    }

    void AttackStart(Unit* who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_22114(Creature* creature)
{
    return new npc_22114AI(creature);
}

struct npc_21387AI : public ScriptedAI
{
    npc_21387AI(Creature* creature) : ScriptedAI(creature) { }

    Timer FireBallTimer;
    bool gavecredit;

    void Reset()
    {
        FireBallTimer.Reset(1);
        gavecredit = false;
        me->RemoveAllAuras();
        me->SetRooted(false);
        ClearCastQueue();
    }

    void SpellHit(Unit* pCaster, const SpellEntry *Spell)
    {
        if(Spell->Id == 38177)
        {
            if(!gavecredit)
            {
                gavecredit = true;
                me->ForcedDespawn(3000);
                me->CastSpell(pCaster, 38178, false);
                me->SetRooted(true);
                FireBallTimer = 0;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (FireBallTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 19816, false);
            FireBallTimer = 3000;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21387(Creature* creature)
{
    return new npc_21387AI(creature);
}

bool GossipHello_npc_22924(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(10990) == QUEST_STATUS_COMPLETE && !player->HasItemCount(32657, 1, true) && 
    ((player->GetQuestStatus(10991) == QUEST_STATUS_AVAILABLE || player->GetQuestStatus(10992) == QUEST_STATUS_AVAILABLE) ||
    (player->GetQuestStatus(10991) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10992) == QUEST_STATUS_INCOMPLETE)))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16433), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_22924(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    switch  (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->CastSpell(player, 40886, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        default: break;
    }

    return true;
}

struct npc_20600AI : public ScriptedAI
{
    npc_20600AI(Creature *c) : ScriptedAI(c)
    {
    }

    bool Enraged;
    Timer MortalWoundTimer;
    Timer BoulderTimer;
    Timer RockRubleTimer;

    void Reset()
    {
        Enraged = false;
        MortalWoundTimer.Reset(1000);
        BoulderTimer.Reset(3000);
        RockRubleTimer.Reset(8000);
    }

    void JustDied(Unit* who)
    {
        who->CastSpell(who, 39891, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(MortalWoundTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 38770, false);
            MortalWoundTimer = 10000;
        }

        if(BoulderTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, false, 0, 8))
                me->CastSpell(target, 38777, false);

            BoulderTimer = 12000;
        }

        if(RockRubleTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, false, 0, 5))
                me->CastSpell(target, 42139, false);

            RockRubleTimer = 8000;
        }

        if(!Enraged && me->HealthBelowPct(30))
        {
            me->CastSpell(me, 40743, false);
            Enraged = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20600(Creature* creature)
{
    return new npc_20600AI(creature);
}


struct npc_20095AI : public ScriptedAI
{
    npc_20095AI(Creature* creature) : ScriptedAI(creature) { }

    Timer YellTimer;

    void Reset()
    {
        YellTimer.Reset(1000);
    }

    void EnterCombat(Unit* who)
    {
        switch (urand(0, 2))
        {
        case 0:
            me->Yell(-1200470, LANG_UNIVERSAL, 0);
            break;
        case 1:
            me->Yell(-1200471, LANG_UNIVERSAL, 0);
            break;
        case 2:
            me->Yell(-1200472, LANG_UNIVERSAL, who->GetGUID());
            break;
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry *Spell)
    {
        if (Spell->Id == 37786)
        {
            switch (urand(0, 2))
            {
            case 0:
                me->Yell(-1200473, LANG_UNIVERSAL, 0);
                break;
            case 1:
                me->Yell(-1200474, LANG_UNIVERSAL, 0);
                break;
            case 2:
                me->Yell(-1200475, LANG_UNIVERSAL, 0);
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (YellTimer.Expired(diff))
            {
                switch (urand(0, 4))
                {
                case 0:
                    me->Yell(-1200476, LANG_UNIVERSAL, 0);
                    break;
                case 1:
                    me->Yell(-1200477, LANG_UNIVERSAL, 0);
                    break;
                case 2:
                    me->Yell(-1200478, LANG_UNIVERSAL, 0);
                    break;
                case 3:
                    me->Yell(-1200479, LANG_UNIVERSAL, 0);
                    break;
                case 4:
                    me->Yell(-1200480, LANG_UNIVERSAL, 0);
                    break;
                }
                YellTimer = 25000;
            }
            return;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20095(Creature* creature)
{
    return new npc_20095AI(creature);
}

struct npc_19957AI : public ScriptedAI
{
    npc_19957AI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {

    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == WAYPOINT_MOTION_TYPE)
        {
            switch (me->GetDBTableGUIDLow())
            {
            case 2382132:
            {
                if (id == 7)
                {
                    me->SetFacingTo(0.3);
                    if (Creature* shama = GetClosestCreatureWithEntry(me, 19992, 10))
                    {
                        shama->SetFacingTo(3.4);
                        shama->HandleEmote(1);
                        me->HandleEmote(1);
                        shama->AI()->EventPulse(me, 1);
                    }
                }
                if (id == 16)
                {
                    if (Creature* brute = GetClosestCreatureWithEntry(me, 19991, 10))
                    {
                        brute->SetFacingTo(5.6);
                        brute->HandleEmote(1);
                        me->HandleEmote(1);
                        brute->AI()->EventPulse(me, 1);
                    }
                }
            }
            case 71172:
            {
                if (id == 4)
                {
                    me->SetFacingTo(4.7);
                    if (Creature* brute = GetClosestCreatureWithEntry(me, 19991, 15))
                    {
                        brute->SetFacingTo(1.6);
                        brute->HandleEmote(1);
                        me->HandleEmote(1);
                        brute->AI()->EventPulse(me, 1);
                    }
                }
                if (id == 13)
                {
                    me->SetFacingTo(3.6);
                    if (Creature* brute = GetClosestCreatureWithEntry(me, 19991, 15))
                    {
                        brute->SetFacingTo(0.6);
                        brute->HandleEmote(1);
                        me->HandleEmote(1);
                        brute->AI()->EventPulse(me, 1);
                    }
                }
                if (id == 17)
                {
                    me->SetFacingTo(4.1);
                    if (Creature* brute = GetClosestCreatureWithEntry(me, 19991, 15))
                    {
                        brute->SetFacingTo(1.1);
                        brute->HandleEmote(1);
                        me->HandleEmote(1);
                        brute->AI()->EventPulse(me, 1);
                    }
                }
            }
            case 71174:
            {
                if (id == 6)
                {
                    me->SetFacingTo(3.3);
                    if (Creature* geomancer = GetClosestCreatureWithEntry(me, 19952, 15))
                    {
                        geomancer->SetFacingTo(0.3);
                        geomancer->HandleEmote(1);
                        me->HandleEmote(1);
                        geomancer->AI()->EventPulse(me, 1);
                    }
                }

                if (id == 26)
                {
                    me->SetFacingTo(6.0);
                    if (Creature* geomancer = GetClosestCreatureWithEntry(me, 19952, 15))
                    {
                        geomancer->SetFacingTo(3.1);
                        geomancer->HandleEmote(1);
                        me->HandleEmote(1);
                        geomancer->AI()->EventPulse(me, 1);
                    }
                }
                if (id == 13)
                {
                    me->SetFacingTo(5.5);
                    if (Creature* geomancer = GetClosestCreatureWithEntry(me, 19952, 15))
                    {
                        geomancer->SetFacingTo(2.4);
                        geomancer->HandleEmote(1);
                        me->HandleEmote(1);
                        geomancer->AI()->EventPulse(me, 1);
                    }
                }
                if (id == 37)
                {
                    me->SetFacingTo(3.08);
                    if (Creature* geomancer = GetClosestCreatureWithEntry(me, 19948, 15))
                    {
                        geomancer->SetFacingTo(6.2);
                        geomancer->HandleEmote(1);
                        me->HandleEmote(1);
                        geomancer->AI()->EventPulse(me, 1);
                    }
                }
            }
            case 71173:
            {
                if (id == 31)
                {
                    me->SetFacingTo(0.3);
                    if (Creature* geomancer = GetClosestCreatureWithEntry(me, 19948, 15))
                    {
                        geomancer->SetFacingTo(3.3);
                        geomancer->HandleEmote(1);
                        me->HandleEmote(1);
                        geomancer->AI()->EventPulse(me, 1);
                    }
                }
            }
            }
        }
    }
};

CreatureAI* GetAI_npc_19957(Creature* creature)
{
    return new npc_19957AI(creature);
}


struct npc_19948AI : public ScriptedAI
{
    npc_19948AI(Creature* creature) : ScriptedAI(creature) {}

    Timer EventTimer;
	Timer EnrageTimer;
    uint32 Phase;

    void Reset()
    {
        EventTimer.Reset(5000);
		EnrageTimer.Reset(10000);
        Phase = 0;
    }

    void EnterCombat(Unit* who)
    {
        EventTimer = 0;
        Phase = 0;
        me->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void UpdateAI(const uint32 diff)
    {
		if (UpdateVictim())
		{
			if (EnrageTimer.Expired(diff))
			{
				me->CastSpell(me, 34932, false);
				EnrageTimer = 0;
			}
			
			DoMeleeAttackIfReady();
		}
		else
        {
            if (EventTimer.Expired(diff))
            {
                switch (Phase)
                {
                case 0:
                    me->HandleEmote(EMOTE_ONESHOT_WAVE);
                    Phase = 1;
                    EventTimer = 6000;
                    break;
                case 1:
                    me->SetStandState(UNIT_STAND_STATE_SIT);
                    Phase = 2;
                    EventTimer = 2000;
                    break;
                case 2:
                    me->CastSpell(me, 46583, false);
                    Phase = 3;
                    EventTimer = 5000;
                    break;
                case 3:
                    me->CastSpell(me, 46583, false);
                    Phase = 4;
                    EventTimer = 5000;
                    break;
                case 4:
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    Phase = 5;
                    EventTimer = 2000;
                    break;
                case 5:
                    //me->CastSpell(me, 34932, false);
                    me->GetMotionMaster()->MoveTargetedHome();
                    Phase = 0;
                    EventTimer = 30000;
                    break;
                }
            }
            return;
        }
    }
};

CreatureAI* GetAI_npc_19948(Creature* creature)
{
    return new npc_19948AI(creature);
}


bool GossipHello_npc_23473(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());
    //HandleWorldEventGossip(player, _Creature);
    if (!isGameEventActive(137))
    {
        std::tm bashirAttack;
        std::time_t now = time(NULL);
        bashirAttack = *std::gmtime(&now);
        uint32 NextBashirAttack = 60 - bashirAttack.tm_min + 60 * (bashirAttack.tm_hour % 2);
        player->SendUpdateWorldState(-4581, NextBashirAttack);
        player->SEND_GOSSIP_MENU(11056, _Creature->GetGUID());
    }
    else player->SEND_GOSSIP_MENU(990034, _Creature->GetGUID());
    return true;
}

/*######
## npc_wyrmcult_zaelot
######*/

#define EMOTE_DEATH_RATTLE                  -1000014
#define EMOTE_FRENZY                        -1269018
#define MORPH_WYRMCULT_BLESSED              12894

#define SPELL_FIREBALL                      9053
#define SPELL_FIRE_NOVA                     11969
#define SPELL_BLESSING_OF_THE_BLACK         37635
#define SPELL_SHOOT_SCOUT                   15547
#define SPELL_POWER_WORD_SHIELD_ACOLYTE     17139
#define SPELL_ENRAGE_ZAELOT                 8599

struct npc_wyrmcult_zaelotAI : public ScriptedAI
{
    npc_wyrmcult_zaelotAI(Creature* creature) : ScriptedAI(creature) {}

    Timer FireballTimer;
    Timer FireNovaTimer;
    Timer BlessingTimer;

    bool dragonPhase;
    bool blessing;
    bool isEnraged;
    
    void Reset()
    {
        FireballTimer.Reset(1);
        FireNovaTimer.Reset(1);
        BlessingTimer = 0;

        dragonPhase = false;
        isEnraged = false;
        blessing = false;

        m_creature->DeMorph();
        ClearCastQueue();

        if (m_creature->HasAura(SPELL_ENRAGE_ZAELOT, 0))
            m_creature->RemoveAurasDueToSpell(SPELL_ENRAGE_ZAELOT);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (!m_creature->HasAura(SPELL_ENRAGE_ZAELOT, 0))
            isEnraged = false;

        if (!dragonPhase)
        {
            if (m_creature->GetHealthPercent() <= 50 && !isEnraged)
            {
                DoScriptText(EMOTE_FRENZY, m_creature);
                m_creature->CastSpell(m_creature, SPELL_ENRAGE_ZAELOT, true);
                
                isEnraged = true;
            }

            if (m_creature->GetHealthPercent() <= 25 && !blessing)
            {
                DoScriptText(EMOTE_DEATH_RATTLE, m_creature);
                m_creature->CastSpell(m_creature, SPELL_BLESSING_OF_THE_BLACK, false);

                blessing = true;
                BlessingTimer = 2000;
            }

            if (BlessingTimer.Expired(diff))
            {
                if (blessing)
                {
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_WYRMCULT_BLESSED);
                    m_creature->SetHealth(m_creature->GetMaxHealth()/4);
                    m_creature->setPowerType(POWER_MANA);
                    m_creature->SetMaxPower(POWER_MANA, 2933);
                    m_creature->SetPower(POWER_MANA, 2933);

                    if (m_creature->HasAura(SPELL_ENRAGE_ZAELOT, 0))
                        m_creature->RemoveAurasDueToSpell(SPELL_ENRAGE_ZAELOT);
                        
                    dragonPhase = true;
                }
            }
        }

        else 
        {
            if (FireNovaTimer.Expired(diff))
            {
                if (m_creature->GetVictim()->GetDistance(m_creature) <= 5)
                {
                    AddSpellToCast(m_creature->GetVictim(), SPELL_FIRE_NOVA, true);
                    FireNovaTimer = urand(6000, 9000);
                }
            }

            if (FireballTimer.Expired(diff))
            {
                AddSpellToCast(m_creature->GetVictim(), SPELL_FIREBALL, false);
                FireballTimer = 3000;
            }
        }
        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wyrmcult_zaelot(Creature* creature)
{
    return new npc_wyrmcult_zaelotAI (creature);
}

/*######
## npc_wyrmcult_acolyte
######*/

struct npc_wyrmcult_acolyteAI : public ScriptedAI
{
    npc_wyrmcult_acolyteAI(Creature* creature) : ScriptedAI(creature) {}

    Timer FireballTimer;
    Timer ShieldTimer;
    Timer FireNovaTimer;
    Timer BlessingTimer;

    bool dragonPhase;
    bool blessing;
    
    void Reset()
    {
        ShieldTimer.Reset(1);
        FireballTimer.Reset(1);
        FireNovaTimer.Reset(1);
        BlessingTimer = 0;

        dragonPhase = false;
        blessing = false;

        m_creature->DeMorph();
        ClearCastQueue();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (!dragonPhase)
        {
            if (m_creature->GetHealthPercent() <= 25 && !blessing)
            {
                m_creature->CastSpell(m_creature, SPELL_BLESSING_OF_THE_BLACK, false);
                DoScriptText(EMOTE_DEATH_RATTLE, m_creature);

                blessing = true;
                BlessingTimer = 2000;
            }

            if (BlessingTimer.Expired(diff))
            {
                if (blessing)
                {
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_WYRMCULT_BLESSED);
                    m_creature->SetHealth(m_creature->GetMaxHealth()/4);
                    m_creature->setPowerType(POWER_MANA);
                    m_creature->SetMaxPower(POWER_MANA, 2933);
                    m_creature->SetPower(POWER_MANA, 2933);

                    if (m_creature->HasAura(SPELL_ENRAGE_ZAELOT, 0))
                        m_creature->RemoveAurasDueToSpell(SPELL_ENRAGE_ZAELOT);
                        
                    dragonPhase = true;
                }
            }

            if (ShieldTimer.Expired(diff))
            {
                AddSpellToCast(m_creature, SPELL_POWER_WORD_SHIELD_ACOLYTE, false);
                ShieldTimer = 30000;
            }
        }

        else 
        {
            if (FireNovaTimer.Expired(diff))
            {
                if (m_creature->GetVictim()->GetDistance(m_creature) <= 5)
                {
                    AddSpellToCast(m_creature->GetVictim(), SPELL_FIRE_NOVA, true);
                    FireNovaTimer = urand(6000, 9000);
                }
            }
        }

        if (FireballTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FIREBALL, false);
            FireballTimer = 3000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wyrmcult_acolyte(Creature* creature)
{
    return new npc_wyrmcult_acolyteAI (creature);
}

/*######
## npc_wyrmcult_blessed
######*/

struct npc_wyrmcult_blessedAI : public ScriptedAI
{
    npc_wyrmcult_blessedAI(Creature* creature) : ScriptedAI(creature) {}

    Timer FireballTimer;
    Timer FireNovaTimer;
    
    void Reset()
    {
        FireballTimer.Reset(1);
        FireNovaTimer.Reset(6000);

        ClearCastQueue();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (FireNovaTimer.Expired(diff))
        {
            if (m_creature->GetVictim()->GetDistance(m_creature) <= 5)
            {
                AddSpellToCast(m_creature->GetVictim(), SPELL_FIRE_NOVA, true);
                FireNovaTimer = urand(6000, 9000);
            }
        }

        if (FireballTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FIREBALL, false);
            FireballTimer = 3000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wyrmcult_blessed(Creature* creature)
{
    return new npc_wyrmcult_blessedAI (creature);
}

/*######
## npc_wyrmcult_scout
######*/

struct npc_wyrmcult_scoutAI : public ScriptedAI
{
    npc_wyrmcult_scoutAI(Creature* creature) : ScriptedAI(creature) {}

    Timer ShootTimer;
    Timer FireballTimer;
    Timer FireNovaTimer;
    Timer BlessingTimer;
    
    bool dragonPhase;
    bool blessing;
    
    void Reset()
    {
        ShootTimer.Reset(1);
        FireballTimer.Reset(1);
        FireNovaTimer.Reset(1);
        BlessingTimer = 0;

        dragonPhase = false;
        blessing = false;

        m_creature->DeMorph();
        ClearCastQueue();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (!dragonPhase)
        {
            if (m_creature->GetHealthPercent() <= 25 && !blessing)
            {
                m_creature->CastSpell(m_creature, SPELL_BLESSING_OF_THE_BLACK, false);
                DoScriptText(EMOTE_DEATH_RATTLE, m_creature);

                blessing = true;
                BlessingTimer = 2000;
            }

            if (BlessingTimer.Expired(diff))
            {
                if (blessing)
                {
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_WYRMCULT_BLESSED);
                    m_creature->SetHealth(m_creature->GetMaxHealth()/4);
                    m_creature->setPowerType(POWER_MANA);
                    m_creature->SetMaxPower(POWER_MANA, 2933);
                    m_creature->SetPower(POWER_MANA, 2933);

                    if (m_creature->HasAura(SPELL_ENRAGE_ZAELOT, 0))
                        m_creature->RemoveAurasDueToSpell(SPELL_ENRAGE_ZAELOT);
                        
                    dragonPhase = true;
                }
            }

            if (ShootTimer.Expired(diff))
            {
                if (m_creature->GetVictim()->GetDistance(m_creature) <= GetSpellMaxRange(SPELL_SHOOT_SCOUT) &&
                    m_creature->GetVictim()->GetDistance(m_creature) >= 5)
                {
                    m_creature->CastSpell(m_creature->GetVictim(), SPELL_SHOOT_SCOUT, false);
                    ShootTimer = 5000;
                }
                
            }
        }

        else 
        {
            if (FireNovaTimer.Expired(diff))
            {
                if (m_creature->GetVictim()->GetDistance(m_creature) <= 5)
                {
                    AddSpellToCast(m_creature->GetVictim(), SPELL_FIRE_NOVA, true);
                    FireNovaTimer = urand(6000, 9000);
                }
            }

            if (FireballTimer.Expired(diff))
            {
                AddSpellToCast(m_creature->GetVictim(), SPELL_FIREBALL, false);
                FireballTimer = 3000;
            }
        }
        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wyrmcult_scout(Creature* creature)
{
    return new npc_wyrmcult_scoutAI (creature);
}

/*######
## AddSC
######*/

void AddSC_blades_edge_mountains()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name="npc_wyrmcult_zaelot";
    newscript->GetAI = &GetAI_npc_wyrmcult_zaelot;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wyrmcult_acolyte";
    newscript->GetAI = &GetAI_npc_wyrmcult_acolyte;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wyrmcult_blessed";
    newscript->GetAI = &GetAI_npc_wyrmcult_blessed;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wyrmcult_scout";
    newscript->GetAI = &GetAI_npc_wyrmcult_scout;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23473";
    newscript->pGossipHello = &GossipHello_npc_23473;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19948";
    newscript->GetAI = &GetAI_npc_19948;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19957";
    newscript->GetAI = &GetAI_npc_19957;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_20095";
    newscript->GetAI = &GetAI_npc_20095;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_nether_drake";
    newscript->GetAI = &GetAI_mobs_nether_drake;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_daranelle";
    newscript->GetAI = &GetAI_npc_daranelle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_overseer_nuaar";
    newscript->pGossipHello = &GossipHello_npc_overseer_nuaar;
    newscript->pGossipSelect = &GossipSelect_npc_overseer_nuaar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_saikkal_the_elder";
    newscript->pGossipHello = &GossipHello_npc_saikkal_the_elder;
    newscript->pGossipSelect = &GossipSelect_npc_saikkal_the_elder;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_skyguard_handler_irena";
    newscript->pGossipHello =  &GossipHello_npc_skyguard_handler_irena;
    newscript->pGossipSelect = &GossipSelect_npc_skyguard_handler_irena;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bloodmaul_brutebane";
    newscript->GetAI = &GetAI_npc_bloodmaul_brutebane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ogre_brute";
    newscript->GetAI = &GetAI_npc_ogre_brute;
    newscript->pReceiveEmote = &ReceiveEmote_npc_ogre_brute;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_vimgol_the_vile";
    newscript->GetAI = &GetAI_npc_vimgol_the_vile;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_vim_bunny";
    newscript->GetAI = &GetAI_npc_vim_bunny;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_aetherray";
    newscript->GetAI = &GetAI_mob_aetherray;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wildlord_antelarion";
    newscript->pGossipHello = &GossipHello_npc_wildlord_antelarion;
    newscript->pGossipSelect = &GossipSelect_npc_wildlord_antelarion;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kolphis_darkscale";
    newscript->pGossipHello = &GossipHello_npc_kolphis_darkscale;
    newscript->pGossipSelect = &GossipSelect_npc_kolphis_darkscale;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_prophecy_trigger";
    newscript->GetAI = &GetAI_npc_prophecy_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_thunderspike";
    newscript->pGOUse = &GOUse_go_thunderspike;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_apexis_relic";
    newscript->pGOUse = &OnGossipHello_go_apexis_relic;
    newscript->pGossipSelectGO = &OnGossipSelect_go_apexis_relic;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_simon_cluster";
    newscript->pGOUse = &OnGossipHello_go_simon_cluster;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_simon_bunny";
    newscript->GetAI = &Get_npc_simon_bunnyAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_orb_attracter";
    newscript->GetAI = &Get_npc_orb_attracterAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_razaan_event";
    newscript->GetAI = &GetAI_npc_razaan_event;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_razaani_raider";
    newscript->GetAI = &GetAI_npc_razaani_raider;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_rally_zapnabber";
    newscript->GetAI = &GetAI_npc_rally_zapnabber;
    newscript->pGossipHello =   &GossipHello_npc_rally_zapnabber;
    newscript->pGossipSelect =  &GossipSelect_npc_rally_zapnabber;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_anger_camp";
    newscript->GetAI = &GetAI_npc_anger_camp;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_obeliks";
    newscript->pGOUse = &GOUse_go_obeliks;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_cannon_target";
    newscript->GetAI = &GetAI_npc_cannon_target;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_gargrom";
    newscript->GetAI = &GetAI_npc_gargrom;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_soulgrinder";
    newscript->GetAI = &GetAI_npc_soulgrinder;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_bashir_landing";
    newscript->GetAI = &GetAI_npc_bashir_landing;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_banishing_crystal";
    newscript->GetAI = &GetAI_npc_banishing_crystal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_oscillating_frequency_scanner_master_bunny";
    newscript->GetAI = &GetAI_npc_oscillating_frequency_scanner_master_bunny;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_scalewing_serpent";
    newscript->GetAI = &GetAI_npc_scalewing_serpent;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23260";
    newscript->GetAI = &GetAI_npc_23260;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23059";
    newscript->GetAI = &GetAI_npc_23059;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23500";
    newscript->GetAI = &GetAI_npc_23500;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23055";
    newscript->GetAI = &GetAI_npc_23055;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23113";
    newscript->GetAI = &GetAI_npc_23113;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23075";
    newscript->GetAI = &GetAI_npc_23075;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23078";
    newscript->GetAI = &GetAI_npc_23078;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23173";
    newscript->GetAI = &GetAI_npc_23173;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23199";
    newscript->GetAI = &GetAI_npc_23199;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23220";
    newscript->GetAI = &GetAI_npc_23220;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20216";
    newscript->GetAI = &GetAI_npc_20216;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_22114";
    newscript->GetAI = &GetAI_npc_22114;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21387";
    newscript->GetAI = &GetAI_npc_21387;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_22924";
    newscript->pGossipHello = &GossipHello_npc_22924;
    newscript->pGossipSelect = &GossipSelect_npc_22924;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20600";
    newscript->GetAI = &GetAI_npc_20600;
    newscript->RegisterSelf();
}
