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
SDName: Karazhan
SD%Complete: 100
SDComment: Support for Berthold (Doorman), Support for Quest 9645.
SDCategory: Karazhan
EndScriptData */

/* ContentData
npc_berthold
npc_image_of_medivh
EndContentData */

#include "precompiled.h"
#include "def_karazhan.h"

/*###
# npc_berthold
####*/

#define SPELL_TELEPORT           39567

#define GOSSIP_ITEM_PLACE        16169
#define GOSSIP_ITEM_MEDIVH       16170
#define GOSSIP_ITEM_TOWER        16171
#define GOSSIP_ITEM_TELEPORT     16172

bool GossipHello_npc_berthold(Player* player, Creature* _Creature)
{
    ScriptedInstance* pInstance = (_Creature->GetInstanceData());
                                                            // Check if Shade of Aran is dead or not
    bool aranDone = false;
    if(pInstance && (pInstance->GetData(DATA_SHADEOFARAN_EVENT) >= DONE))
        aranDone = true;

    _Creature->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
    
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_PLACE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_MEDIVH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_TOWER), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    if (aranDone)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_TELEPORT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

    if (aranDone)
        player->SEND_GOSSIP_MENU(25044, _Creature->GetGUID());
    else
        player->SEND_GOSSIP_MENU(25035, _Creature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_berthold(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    _Creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);

    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->SEND_GOSSIP_MENU(25036, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->SEND_GOSSIP_MENU(25037, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->SEND_GOSSIP_MENU(25038, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
            player->CastSpell(player, SPELL_TELEPORT, true);
            player->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}


/*###
# npc_calliard
####*/

#define GOSSIP_ITEM_MIDNIGHT    16173

#define CALLIARD_SAY1           -1200246
#define CALLIARD_SAY2           -1200247
#define CALLIARD_SAY3           -1200248

struct npc_calliardAI : public ScriptedAI
{
    npc_calliardAI(Creature* c) : ScriptedAI(c) {}

    Timer SayTimer;

    void Reset()
    {
        SayTimer.Reset(60000);
    }

    void UpdateAI(const uint32 diff)
    {
        
        if (SayTimer.Expired(diff))
        {
            me->Say(RAND(-1200246, -1200247, -1200248), 0, 0);
            SayTimer = urand(60000, 180000);
        }
    }
};

CreatureAI* GetAI_npc_calliard(Creature *_Creature)
{
    return new npc_calliardAI(_Creature);
}

bool GossipHello_npc_calliard(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_MIDNIGHT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(25042, _Creature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_calliard(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->SEND_GOSSIP_MENU(25043, _Creature->GetGUID());
            break;
    }
    return true;
}

/*###
# npc_hastings
####*/

#define GOSSIP_ITEM_HELP    16174
#define GOSSIP_ITEM_BIG     16175

bool GossipHello_npc_hastings(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_HELP), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(25039, _Creature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_hastings(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_BIG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(25040, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->SEND_GOSSIP_MENU(25041, _Creature->GetGUID());
            break;
    }
    return true;
}




/*###
# npc_image_of_medivh
####*/

#define SAY_DIALOG_MEDIVH_1         -1200249
#define SAY_DIALOG_ARCANAGOS_2      -1200250
#define SAY_DIALOG_MEDIVH_3         -1200251
#define SAY_DIALOG_ARCANAGOS_4      -1200252
#define SAY_DIALOG_MEDIVH_5         -1200253
#define SAY_DIALOG_ARCANAGOS_6      -1200254
#define EMOTE_DIALOG_MEDIVH_7       -1200255
#define SAY_DIALOG_ARCANAGOS_8      -1200256
#define SAY_DIALOG_MEDIVH_9         -1200257

#define MOB_ARCANAGOS               17652
#define SPELL_FIRE_BALL             30967
#define SPELL_UBER_FIREBALL         30971
#define SPELL_CONFLAGRATION_BLAST   30977
#define SPELL_MANA_SHIELD           31635

static float MedivPos[4] = {-11161.49,-1902.24,91.48,1.94};
static float ArcanagosPos[4] = {-11169.75,-1881.48,95.39,4.83};

struct npc_image_of_medivhAI : public ScriptedAI
{
    npc_image_of_medivhAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    uint64 ArcanagosGUID;

    Timer_UnCheked YellTimer;
    uint32 Step;
    Timer_UnCheked FireMedivhTimer;
    Timer_UnCheked FireArcanagosTimer;

    bool EventStarted;

    void Reset()
    {
        ArcanagosGUID = 0;

        if(pInstance && pInstance->GetData64(DATA_IMAGE_OF_MEDIVH) == 0)
        {
            pInstance->SetData64(DATA_IMAGE_OF_MEDIVH, m_creature->GetGUID());
            (*m_creature).GetMotionMaster()->MovePoint(1,MedivPos[0],MedivPos[1],MedivPos[2]);
            Step = 0;
        }else
        {
            m_creature->DealDamage(m_creature,m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();
        }
    }
    void EnterCombat(Unit* who){}

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;
        if(id == 1)
        {
            StartEvent();
            m_creature->SetOrientation(MedivPos[3]);
            m_creature->SetOrientation(MedivPos[3]);
        }
    }

    void StartEvent()
    {
        Step = 1;
        EventStarted = true;
        Creature* Arcanagos = m_creature->SummonCreature(MOB_ARCANAGOS,ArcanagosPos[0],ArcanagosPos[1],ArcanagosPos[2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,20000);
        if(!Arcanagos)
            return;
        ArcanagosGUID = Arcanagos->GetGUID();
        Arcanagos->SetLevitate(true);
        (*Arcanagos).GetMotionMaster()->MovePoint(0,ArcanagosPos[0],ArcanagosPos[1],ArcanagosPos[2]);
        Arcanagos->SetOrientation(ArcanagosPos[3]);
        m_creature->SetOrientation(MedivPos[3]);
        YellTimer = 10000;
    }


    uint32 NextStep(uint32 Step)
    {
        Unit* arca = Unit::GetUnit((*m_creature),ArcanagosGUID);
        Map *map = m_creature->GetMap();
        switch(Step)
        {
            case 0: return 9999999;
            case 1:
                m_creature->Yell(-1200249, LANG_UNIVERSAL, 0);
                return 10000;
            case 2:
                if(arca)
                    ((Creature*)arca)->Yell(-1200250, LANG_UNIVERSAL, 0);
                return 20000;
            case 3:
                m_creature->Yell(-1200251,LANG_UNIVERSAL,0);
                return 10000;
            case 4:
                if(arca)
                    ((Creature*)arca)->Yell(-1200252, LANG_UNIVERSAL, 0);
                return 20000;
            case 5:
                m_creature->Yell(-1200253, LANG_UNIVERSAL, 0);
                return 20000;
            case 6:
                if(arca)
                    ((Creature*)arca)->Yell(-1200254, LANG_UNIVERSAL, 0);
                return 10000;
            case 7:
                FireArcanagosTimer = 500;
                return 5000;
            case 8:
                FireMedivhTimer = 500;
                DoCast(m_creature, SPELL_MANA_SHIELD);
                return 10000;
            case 9:
                m_creature->TextEmote(-1200255, 0, false);
                return 10000;
            case 10:
                if(arca)
                    m_creature->CastSpell(arca, SPELL_CONFLAGRATION_BLAST, false);
                return 1000;
            case 11:
                if(arca)
                    ((Creature*)arca)->Yell(-1200256, LANG_UNIVERSAL, 0);
                return 5000;
            case 12:
                arca->GetMotionMaster()->MovePoint(0, -11010.82,-1761.18, 156.47);
                arca->setActive(true);
                arca->InterruptNonMeleeSpells(true);
                arca->SetSpeed(MOVE_FLIGHT, 2.0f);
                return 10000;
            case 13:
                m_creature->Yell(-1200257, LANG_UNIVERSAL, 0);
                return 10000;
            case 14:
                m_creature->SetVisibility(VISIBILITY_OFF);
                m_creature->ClearInCombat();

                if(map->IsDungeon())
                {
                    InstanceMap::PlayerList const &PlayerList = ((InstanceMap*)map)->GetPlayers();
                    for (InstanceMap::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    {
                        if(i->getSource()->isAlive())
                        {
                            if(i->getSource()->GetQuestStatus(9645) == QUEST_STATUS_INCOMPLETE)
                                i->getSource()->CompleteQuest(9645);
                        }
                    }
                }
                return 50000;
            case 15:
                arca->DealDamage(arca,arca->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                return 5000;
            default : return 9999999;
        }

    }

    void UpdateAI(const uint32 diff)
    {
        
        if (YellTimer.Expired(diff))
        {
            if(EventStarted)
            {
                YellTimer = NextStep(Step++);
            }
        }

        if(Step >= 7 && Step <= 12 )
        {
            Unit* arca = Unit::GetUnit((*m_creature),ArcanagosGUID);

     
            if (FireArcanagosTimer.Expired(diff))
            {
                if(arca)
                    arca->CastSpell(m_creature, SPELL_FIRE_BALL, false);
                FireArcanagosTimer = 6000;
            }

            if (FireMedivhTimer.Expired(diff))
            {
                if(arca)
                    DoCast(arca, SPELL_FIRE_BALL);
                FireMedivhTimer = 5000;
            }

        }
    }
};

CreatureAI* GetAI_npc_image_of_medivh(Creature *_Creature)
{
    return new npc_image_of_medivhAI(_Creature);
}

void AddSC_karazhan()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_berthold";
    newscript->pGossipHello = &GossipHello_npc_berthold;
    newscript->pGossipSelect = &GossipSelect_npc_berthold;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_calliard";
    newscript->GetAI = &GetAI_npc_calliard;
    newscript->pGossipHello = &GossipHello_npc_calliard;
    newscript->pGossipSelect = &GossipSelect_npc_calliard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_hastings";
    newscript->pGossipHello = &GossipHello_npc_hastings;
    newscript->pGossipSelect = &GossipSelect_npc_hastings;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_image_of_medivh";
    newscript->GetAI = &GetAI_npc_image_of_medivh;
    newscript->RegisterSelf();
}
