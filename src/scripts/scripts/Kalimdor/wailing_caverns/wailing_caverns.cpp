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
SDName: Wailing Caverns
SD%Complete: 99
SDComment: Everything seems to work, check timers for spells
SDCategory: Wailing Caverns
EndScriptData */

/* ContentData
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "def_wailing_caverns.h"

/*######
## npc_disciple_of_naralex
######*/

enum eEnums
{
    //say
    SAY_MAKE_PREPARATIONS         = -1043001,
    SAY_TEMPLE_OF_PROMISE         = -1043002,
    SAY_MUST_CONTINUE             = -1043003,
    SAY_BANISH_THE_SPIRITS        = -1043004,
    SAY_CAVERNS_PURIFIED          = -1043005,
    SAY_BEYOND_THIS_CORRIDOR      = -1043006,
    SAY_EMERALD_DREAM             = -1043007,
    SAY_MUTANUS_THE_DEVOURER      = -1043012,
    SAY_NARALEX_AWAKES            = -1043014,
    SAY_THANK_YOU                 = -1043015,
    SAY_FAREWELL                  = -1043016,
    SAY_ATTACKED                  = -1043017,
    //yell
    SAY_AT_LAST                   = -1043000,
    SAY_I_AM_AWAKE                = -1043013,
    //emote
    EMOTE_AWAKENING_RITUAL        = -1043008,
    EMOTE_TROUBLED_SLEEP          = -1043009,
    EMOTE_WRITHE_IN_AGONY         = -1043010,
    EMOTE_HORRENDOUS_VISION       = -1043011,
    //spell
    SPELL_MARK_OF_THE_WILD_RANK_2 = 5232,
    SPELL_SERPENTINE_CLEANSING    = 6270,
    SPELL_NARALEXS_AWAKENING      = 6271,
    SPELL_DRUIDS_POTION           = 8141,
    SPELL_SLEEP                   = 1090,
    SPELL_FLIGHT_FORM             = 33943,
    //npc entry
    NPC_DEVIATE_RAVAGER           = 3636,
    NPC_DEVIATE_VIPER             = 5755,
    NPC_DEVIATE_MOCCASIN          = 5762,
    NPC_NIGHTMARE_ECTOPLASM       = 5763,
    NPC_MUTANUS_THE_DEVOURER      = 3654,
};

#define GOSSIP_ID_START_1       698  //Naralex sleeps again!
#define GOSSIP_ID_START_2       699  //The fanglords are dead!
#define GOSSIP_ITEM_NARALEX     16389

struct npc_disciple_of_naralexAI : public npc_escortAI
{
    npc_disciple_of_naralexAI(Creature *c) : npc_escortAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
        eventTimer = 0;
        currentEvent = 0;
        eventProgress = 0;
        me->setActive(true);
    }

    Timer_UnCheked eventTimer;
    uint32 currentEvent;
    uint32 eventProgress;
    Timer_UnCheked sleepTimer;
    Timer_UnCheked potionTimer;

    bool potCooldown;
    ScriptedInstance *pInstance;

    void WaypointReached(uint32 i)
    {
        if (!pInstance)
            return;

        switch (i)
        {
            case 4:
                eventTimer.Reset(1);
                eventProgress = 1;
                currentEvent = TYPE_NARALEX_PART1;
                pInstance->SetData(TYPE_NARALEX_PART1, IN_PROGRESS);
            break;
            case 5:
                DoScriptText(SAY_MUST_CONTINUE, me);
                pInstance->SetData(TYPE_NARALEX_PART1, DONE);
            break;
            case 11:
                eventTimer.Reset(1);
                eventProgress = 1;
                currentEvent = TYPE_NARALEX_PART2;
                pInstance->SetData(TYPE_NARALEX_PART2, IN_PROGRESS);
            break;
            case 19:
                DoScriptText(SAY_BEYOND_THIS_CORRIDOR, me);
            break;
            case 24:
                eventTimer.Reset(1);
                eventProgress = 1;
                currentEvent = TYPE_NARALEX_PART3;
                pInstance->SetData(TYPE_NARALEX_PART3, IN_PROGRESS);
            break;
        }
    }

    void Reset()
    {
        sleepTimer.Reset(urand(5000, 15000));
        potionTimer.Reset(120000);
        potCooldown = false;
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_ATTACKED, me, who);
    }

    void EnterEvadeMode()
    {
        // Do not stop casting
        if (eventTimer.GetInterval())
        {
            m_creature->SetLootRecipient(NULL);
            m_creature->DeleteThreatList();
            m_creature->CombatStop(false);
        }
        else
            npc_escortAI::EnterEvadeMode();
    }

    void JustDied(Unit * /*slayer*/)
    {
        if (pInstance)
        {
            pInstance->SetData(TYPE_NARALEX_EVENT, FAIL);
            pInstance->SetData(TYPE_NARALEX_PART1, FAIL);
            pInstance->SetData(TYPE_NARALEX_PART2, FAIL);
            pInstance->SetData(TYPE_NARALEX_PART3, FAIL);
        }
    }

    void JustSummoned(Creature* summoned)
    {
         summoned->AI()->AttackStart(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (currentEvent != TYPE_NARALEX_PART3)
            npc_escortAI::UpdateAI(diff);
        
        if (potCooldown)     // 2 mins cooldown on healing potion
        {
            if (potionTimer.Expired(diff))
                potCooldown = false;
        }

        if (!pInstance)
            return;

        if (!UpdateVictim())
        {
            if (eventTimer.Expired(diff))
            {
                if (pInstance->GetData(currentEvent) == IN_PROGRESS)
                {
                    switch (currentEvent)
                    {
                    case TYPE_NARALEX_PART1:
                        if (eventProgress == 1)
                        {
                            ++eventProgress;
                            DoScriptText(SAY_TEMPLE_OF_PROMISE, me);
                            me->SummonCreature(NPC_DEVIATE_RAVAGER, -82.1763, 227.874, -93.3233, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DEVIATE_RAVAGER, -72.9506, 216.645, -93.6756, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            eventTimer = 0;
                        }
                        break;
                    case TYPE_NARALEX_PART2:
                        if (eventProgress == 1)
                        {
                            ++eventProgress;
                            DoScriptText(SAY_BANISH_THE_SPIRITS, me);
                            DoCast(me, SPELL_SERPENTINE_CLEANSING);
                            eventTimer = 30000;
                            me->SummonCreature(NPC_DEVIATE_VIPER, -61.5261, 273.676, -92.8442, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DEVIATE_VIPER, -58.4658, 280.799, -92.8393, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DEVIATE_VIPER, -50.002, 278.578, -92.8442, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                        }
                        else if (eventProgress == 2)
                        {
                            DoScriptText(SAY_CAVERNS_PURIFIED, me);
                            pInstance->SetData(TYPE_NARALEX_PART2, DONE);
                            if (me->HasAura(SPELL_SERPENTINE_CLEANSING, 0))
                                me->RemoveAurasDueToSpell(SPELL_SERPENTINE_CLEANSING);
                            eventTimer = 0;
                        }
                        break;
                    case TYPE_NARALEX_PART3:
                        switch (eventProgress)
                        {
                        case 1:
                            eventTimer = 4000;
                            me->SetStandState(UNIT_STAND_STATE_KNEEL);
                            DoScriptText(SAY_EMERALD_DREAM, me);
                            break;
                        case 2:
                            eventTimer = 15000;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                DoCast(naralex, SPELL_NARALEXS_AWAKENING, true);
                            DoScriptText(EMOTE_AWAKENING_RITUAL, me);
                            break;
                        case 3:
                            eventTimer = 15000;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                DoScriptText(EMOTE_TROUBLED_SLEEP, naralex);
                            me->SummonCreature(NPC_DEVIATE_MOCCASIN, 131.486, 218.504, -101.094, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_DEVIATE_MOCCASIN, 146.517, 233.378, -100.830, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_DEVIATE_MOCCASIN, 135.440, 253.692, -100.067, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            break;
                        case 4:
                            eventTimer = 30000;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                DoScriptText(EMOTE_WRITHE_IN_AGONY, naralex);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 131.486, 218.504, -101.094, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 146.517, 233.378, -100.830, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 135.440, 253.692, -100.067, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 144.752, 243.089, -100.219, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 127.191, 261.635, -99.542, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 117.289, 267.770, -99.667, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            me->SummonCreature(NPC_NIGHTMARE_ECTOPLASM, 101.216, 260.543, -99.718, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
                            break;
                        case 5:
                            eventTimer = 3000;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                DoScriptText(EMOTE_HORRENDOUS_VISION, naralex);
                            me->SummonCreature(NPC_MUTANUS_THE_DEVOURER, 144.752, 243.089, -100.219, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300000);
                            DoScriptText(SAY_MUTANUS_THE_DEVOURER, me);
                            pInstance->SetData(TYPE_MUTANUS_THE_DEVOURER, IN_PROGRESS);
                            break;
                        case 6:
                            eventTimer = 3000;
                            if (pInstance->GetData(TYPE_MUTANUS_THE_DEVOURER) == DONE)
                            {
                                if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                {
                                    if (me->HasAura(SPELL_NARALEXS_AWAKENING, 0))
                                        me->RemoveAurasDueToSpell(SPELL_NARALEXS_AWAKENING);
                                    naralex->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_I_AM_AWAKE, naralex);
                                }
                                DoScriptText(SAY_NARALEX_AWAKES, me);
                            }
                            break;
                        case 7:
                            eventTimer = 6000;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                DoScriptText(SAY_THANK_YOU, naralex);
                            break;
                        case 8:
                            eventTimer = 8000;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                            {
                                DoScriptText(SAY_FAREWELL, naralex);
                                naralex->AddAura(SPELL_FLIGHT_FORM, naralex);
                            }
                            SetRun();
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            me->AddAura(SPELL_FLIGHT_FORM, me);
                            break;
                        case 9:
                            eventTimer = 1500;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                naralex->GetMotionMaster()->MovePoint(25, naralex->GetPositionX(), naralex->GetPositionY(), naralex->GetPositionZ());
                            break;
                        case 10:
                            eventTimer = 2500;
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                            {
                                naralex->GetMotionMaster()->MovePoint(0, 117.095512, 247.107971, -96.167870);
                                naralex->GetMotionMaster()->MovePoint(1, 90.388809, 276.135406, -83.389801);
                            }
                            me->GetMotionMaster()->MovePoint(26, 117.095512, 247.107971, -96.167870);
                            me->GetMotionMaster()->MovePoint(27, 144.375443, 281.045837, -82.477135);
                            break;
                        case 11:
                            if (Creature* naralex = pInstance->instance->GetCreature(pInstance->GetData64(DATA_NARALEX)))
                                naralex->SetVisibility(VISIBILITY_OFF);
                            me->SetVisibility(VISIBILITY_OFF);
                            pInstance->SetData(TYPE_NARALEX_PART3, DONE);
                            eventTimer = 0;
                            break;
                        }
                        if (eventProgress != 6 || pInstance->GetData(TYPE_MUTANUS_THE_DEVOURER) == DONE)
                            eventProgress++;
                        break;
                    }
                }
            }
            return;
        } // if !UpdateVictim()

        if (sleepTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0, true))
                AddSpellToCast(target, SPELL_SLEEP);
            sleepTimer = urand(32000, 40000);
        }
           

        if(me->GetHealthPercent() < 30 && !potCooldown)
        {
            AddSpellToCast(m_creature, SPELL_DRUIDS_POTION);
            potionTimer = 120000;
            potCooldown = true;
        }
    }
};

CreatureAI* GetAI_npc_disciple_of_naralex(Creature* pCreature)
{
    return new npc_disciple_of_naralexAI(pCreature);
}

bool GossipHello_npc_disciple_of_naralex(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance *pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

    if (pInstance)
    {
        pCreature->CastSpell(pPlayer, SPELL_MARK_OF_THE_WILD_RANK_2, true);
        if ((pInstance->GetData(TYPE_LORD_COBRAHN) == DONE) && (pInstance->GetData(TYPE_LORD_PYTHAS) == DONE) &&
            (pInstance->GetData(TYPE_LADY_ANACONDRA) == DONE) && (pInstance->GetData(TYPE_LORD_SERPENTIS) == DONE))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NARALEX), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_START_2, pCreature->GetGUID());

            if (!pInstance->GetData(TYPE_NARALEX_YELLED))
            {
                DoScriptText(SAY_AT_LAST, pCreature);
                pInstance->SetData(TYPE_NARALEX_YELLED, 1);
            }
        }
        else
        {
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_START_1, pCreature->GetGUID());
        }
    }
    return true;
}

bool GossipSelect_npc_disciple_of_naralex(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    ScriptedInstance *pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        if (pInstance)
            pInstance->SetData(TYPE_NARALEX_EVENT, IN_PROGRESS);

        DoScriptText(SAY_MAKE_PREPARATIONS, pCreature);

        pCreature->setFaction(250);

        CAST_AI(npc_escortAI, (pCreature->AI()))->Start(false, true, pPlayer->GetGUID());
        CAST_AI(npc_escortAI, (pCreature->AI()))->SetDespawnAtFar(false);
        CAST_AI(npc_escortAI, (pCreature->AI()))->SetDespawnAtEnd(false);
    }
    return true;
}

void AddSC_wailing_caverns()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_disciple_of_naralex";
    newscript->pGossipHello =  &GossipHello_npc_disciple_of_naralex;
    newscript->pGossipSelect = &GossipSelect_npc_disciple_of_naralex;
    newscript->GetAI = &GetAI_npc_disciple_of_naralex;
    newscript->RegisterSelf();
}
