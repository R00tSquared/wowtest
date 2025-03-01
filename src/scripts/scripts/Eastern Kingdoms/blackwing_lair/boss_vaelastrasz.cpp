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
SDName: Boss_Vaelastrasz
SD%Complete: 75
SDComment: Burning Adrenaline not correctly implemented in core, looks like the loot is missing or connected with other NPC
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define SAY_LINE1           -1469026
#define SAY_LINE2           -1469027
#define SAY_LINE3           -1469028
#define SAY_HALFLIFE        -1469029
#define SAY_KILLTARGET      -1469030

#define GOSSIP_ITEM         16113
#define GOSSIP_ITEM_2       16114

#define SPELL_ESSENCEOFTHERED       23513
#define SPELL_FLAMEBREATH           23461
#define SPELL_FIRENOVA              23462
#define SPELL_TAILSWIPE             15847
#define SPELL_BURNINGADRENALINE     23620
#define SPELL_CLEAVE                20684                   //Chain cleave is most likely named something different and contains a dummy effect

#define QUEST_NEFARIUS_CORRUPTION    8730
#define QUEST_THE_CHARGE_OF_THE_DRAGONFLIGHTS    8555

struct boss_vaelAI : public ScriptedAI
{
    boss_vaelAI(Creature *c) : ScriptedAI(c)
    {
        c->SetUInt32Value(UNIT_NPC_FLAGS,1);
        c->setFaction(35);
        c->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    uint64 PlayerGUID;
    Timer _ChangedNameSpeachTimer;
    uint32 SpeachNum;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameFlameBreath_Timer;
    Timer _ChangedNameFireNova_Timer;
    Timer _ChangedNameBurningAdrenalineCaster_Timer;
    Timer _ChangedNameBurningAdrenalineTank_Timer;
    Timer _ChangedNameTailSwipe_Timer;
    bool HasYelled;
    bool DoingSpeach;

    void Reset()
    {
        PlayerGUID = 0;
        _ChangedNameSpeachTimer.Reset(1);
        SpeachNum = 0;
        _ChangedNameCleave_Timer.Reset(8000);                                //These times are probably wrong
        _ChangedNameFlameBreath_Timer.Reset(11000);
        _ChangedNameBurningAdrenalineCaster_Timer.Reset(15000);
        _ChangedNameBurningAdrenalineTank_Timer.Reset(45000);
        _ChangedNameFireNova_Timer.Reset(5000);
        _ChangedNameTailSwipe_Timer.Reset(20000);
        HasYelled = false;
        DoingSpeach = false;

        if (pInstance && pInstance->GetData(DATA_VAELASTRASZ_THE_CORRUPT_EVENT) != DONE)
            pInstance->SetData(DATA_VAELASTRASZ_THE_CORRUPT_EVENT, NOT_STARTED);
    }

    void BeginSpeach(Unit* target)
    {
        //Stand up and begin speach
        PlayerGUID = target->GetGUID();

        //10 seconds
        DoScriptText(SAY_LINE1, m_creature);

        _ChangedNameSpeachTimer.Reset(10000);
        SpeachNum = 0;
        DoingSpeach = true;

        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void KilledUnit(Unit *victim)
    {
        if (rand()%5)
            return;

        DoScriptText(SAY_KILLTARGET, m_creature, victim);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_VAELASTRASZ_THE_CORRUPT_EVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        DoCast(m_creature,SPELL_ESSENCEOFTHERED);
        DoZoneInCombat();
        m_creature->SetHealth(int(m_creature->GetMaxHealth()*.3));
        m_creature->ResetPlayerDamageReq();

        if (pInstance)
            pInstance->SetData(DATA_VAELASTRASZ_THE_CORRUPT_EVENT, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        //Speach
        if (DoingSpeach)
        {
            if (_ChangedNameSpeachTimer.Expired(diff))
            {
                switch (SpeachNum)
                {
                    case 0:
                        //16 seconds till next line
                        DoScriptText(SAY_LINE2, m_creature);
                        _ChangedNameSpeachTimer = 16000;
                        SpeachNum++;
                        break;
                    case 1:
                        //This one is actually 16 seconds but we only go to 10 seconds because he starts attacking after he says "I must fight this!"
                        DoScriptText(SAY_LINE3, m_creature);
                        _ChangedNameSpeachTimer = 10000;
                        SpeachNum++;
                        break;
                    case 2:
                        m_creature->setFaction(103);
                        if (PlayerGUID && Unit::GetUnit((*m_creature),PlayerGUID))
                        {
                            AttackStart(Unit::GetUnit((*m_creature),PlayerGUID));
                            DoCast(m_creature,SPELL_ESSENCEOFTHERED);
                        }
                        _ChangedNameSpeachTimer.Reset(1);
                        DoingSpeach = false;
                        break;
                }
            }
        }

        //Return since we have no target
        if (!UpdateVictim() )
            return;

        // Yell if hp lower than 15%
        if (me->GetHealthPercent() < 15 && !HasYelled)
        {
            DoScriptText(SAY_HALFLIFE, m_creature);
            HasYelled = true;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 15000;
        }

        if (_ChangedNameFlameBreath_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FLAMEBREATH);
            _ChangedNameFlameBreath_Timer = 4000 + rand()%4000;
        }

        if (_ChangedNameBurningAdrenalineCaster_Timer.Expired(diff))
        {
            Unit* target = NULL;

            int i = 0 ;
            while (i < 3)                                   // max 3 tries to get a random target with power_mana
            {
                ++i;
                target = SelectUnit(SELECT_TARGET_RANDOM,1);//not aggro leader
                if (target)
                    if (target->getPowerType() == POWER_MANA)
                        i=3;
            }
            if (target)                                     // cast on self (see below)
                target->CastSpell(target,SPELL_BURNINGADRENALINE,1);

            _ChangedNameBurningAdrenalineCaster_Timer = 15000;
        }

        if (_ChangedNameBurningAdrenalineTank_Timer.Expired(diff))
        {
            // have the victim cast the spell on himself otherwise the third effect aura will be applied
            // to Vael instead of the player
            m_creature->GetVictim()->CastSpell(m_creature->GetVictim(),SPELL_BURNINGADRENALINE,1);

            _ChangedNameBurningAdrenalineTank_Timer = 45000;
        }

        if (_ChangedNameFireNova_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIRENOVA);
            _ChangedNameFireNova_Timer = 5000;
        }

        if (_ChangedNameTailSwipe_Timer.Expired(diff))
        {
            //Only cast if we are behind
            /*if (!m_creature->HasInArc( M_PI, m_creature->GetVictim()))
            {
            DoCast(m_creature->GetVictim(),SPELL_TAILSWIPE);
            }*/

            _ChangedNameTailSwipe_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};

void SendDefaultMenu_boss_vael(Player *player, Creature *_Creature, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)               //Fight time
    {
        player->CLOSE_GOSSIP_MENU();
        ((boss_vaelAI*)_Creature->AI())->BeginSpeach((Unit*)player);
    }
    else if (action == GOSSIP_ACTION_INFO_DEF + 2)
    {
        player->PrepareQuestMenu(_Creature->GetGUID());
        player->SendPreparedQuest(_Creature->GetGUID());
    }
}

bool GossipSelect_boss_vael(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (sender == GOSSIP_SENDER_MAIN)
        SendDefaultMenu_boss_vael(player, _Creature, action);

    return true;
}

bool GossipHello_boss_vael(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM)        , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    if ((player->GetQuestStatus(QUEST_THE_CHARGE_OF_THE_DRAGONFLIGHTS) == QUEST_STATUS_COMPLETE) && (player->GetQuestStatus(QUEST_NEFARIUS_CORRUPTION) == QUEST_STATUS_NONE))
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->SEND_GOSSIP_MENU(907,_Creature->GetGUID());

    return true;
}

CreatureAI* GetAI_boss_vael(Creature *_Creature)
{
    return new boss_vaelAI (_Creature);
}

void AddSC_boss_vael()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_vaelastrasz";
    newscript->GetAI = &GetAI_boss_vael;
    newscript->pGossipHello = &GossipHello_boss_vael;
    newscript->pGossipSelect = &GossipSelect_boss_vael;
    newscript->RegisterSelf();
}


