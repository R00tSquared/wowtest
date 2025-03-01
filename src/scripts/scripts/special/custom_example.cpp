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
SDName: Custom_Example
SD%Complete: 100
SDComment: Short custom scripting example
SDCategory: Script Examples
EndScriptData */

#include "precompiled.h"

// **** This script is designed as an example for others to build on ****
// **** Please modify whatever you'd like to as this script is only for developement ****

// **** Script Info ****
// This script is written in a way that it can be used for both friendly and hostile monsters
// Its primary purpose is to show just how much you can really do with scripts
// I recommend trying it out on both an agressive NPC and on friendly npc

// **** Quick Info ****
// Functions with Handled Function marked above them are functions that are called automatically by the core
// Functions that are marked Custom Function are functions I've created to simplify code

#define SPELL_BUFF      25661
#define SPELL_ONE       12555
#define SPELL_ONE_ALT   24099
#define SPELL_TWO       10017
#define SPELL_THREE     26027
#define SPELL_ENRAGE    23537
#define SPELL_BESERK    32309

#define SAY_AGGRO       -1200184
#define SAY_RANDOM_0    -1200185
#define SAY_RANDOM_1    -1200186
#define SAY_RANDOM_2    -1200187
#define SAY_RANDOM_3    -1200188
#define SAY_RANDOM_4    -1200189
#define SAY_BESERK      -1200190
#define SAY_PHASE       -1200191

#define GOSSIP_ITEM     16088
#define SAY_DANCE       -1200192
#define SAY_SALUTE      -1200193

struct custom_exampleAI : public ScriptedAI
{
    //*** HANDLED FUNCTION ***
    //This is the constructor, called only once when the creature is first created
    custom_exampleAI(Creature *c) : ScriptedAI(c) {}

    //*** CUSTOM VARIABLES ****
    //These variables are for use only by this individual script.
    //Nothing else will ever call them but us.

    Timer_UnCheked Say_Timer;                                       //Timer for random chat
    Timer_UnCheked Rebuff_Timer;                                    //Timer for rebuffing
    Timer_UnCheked Spell_1_Timer;                                   //Timer for spell 1 when in combat
    Timer_UnCheked Spell_2_Timer;                                   //Timer for spell 1 when in combat
    Timer_UnCheked Spell_3_Timer;                                   //Timer for spell 1 when in combat
    Timer_UnCheked Beserk_Timer;                                    //Timer until we go into Beserk (enraged) mode
    uint32 Phase;                                          //The current battle phase we are in
    Timer_UnCheked Phase_Timer;                                     //Timer until phase transition

    //*** HANDLED FUNCTION ***
    //This is called whenever the core decides we need to evade
    void Reset()
    {
        Phase = 1;                                          //Start in phase 1
        Phase_Timer.Reset(60000);                                //60 seconds
        Spell_1_Timer.Reset(5000);                               //5 seconds
        Spell_2_Timer.Reset(37000);                              //37 seconds
        Spell_3_Timer.Reset(19000);                              //19 seconds
        Beserk_Timer.Reset(120000);                              //2 minutes
    }

    //*** HANDLED FUNCTION ***
    //Attack Start is called whenever someone hits us.
    void EnterCombat(Unit *who)
    {
        //Say some stuff
        DoSay(-1200184,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,8280);
    }

    //*** HANDLED FUNCTION ***
    //Update AI is called Every single map update (roughly once every 100ms if a player is within the grid)
    void UpdateAI(const uint32 diff)
    {
        //Out of combat timers
        if (!m_creature->GetVictim())
        {
            //Random Say timer
            if (Say_Timer.Expired(diff))
            {
                //Random switch between 5 outcomes
                switch (rand()%5)
                {
                    case 0:
                        DoYell(-1200185,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8831);  //8831 is the index of the sound we are playing. You find these numbers in SoundEntries.dbc
                        break;

                    case 1:
                        DoYell(-1200186,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8818);
                        break;

                    case 2:
                        DoYell(-1200187,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8041);
                        break;

                    case 3:
                        DoYell(-1200188,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8581);
                        break;

                    case 4:
                        DoYell(-1200189,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8791);
                        break;
                }

                Say_Timer = 45000;                          //Say something agian in 45 seconds
            }

            //Rebuff timer
            if (Rebuff_Timer.Expired(diff))
            {
                DoCast(m_creature,SPELL_BUFF);
                Rebuff_Timer = 900000;                      //Rebuff agian in 15 minutes
            }
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Spell 1 timer
        if (Spell_1_Timer.Expired(diff))
        {
            //Cast spell one on our current target.
            if (rand()%50 > 10)
                DoCast(m_creature->GetVictim(),SPELL_ONE_ALT);
            else
                if (m_creature->IsWithinDistInMap(m_creature->GetVictim(), 25))
                    DoCast(m_creature->GetVictim(),SPELL_ONE);

            Spell_1_Timer = 5000;
        }

        //Spell 2 timer
        if (Spell_2_Timer.Expired(diff))
        {
            //Cast spell one on our current target.
            DoCast(m_creature->GetVictim(),SPELL_TWO);

            Spell_2_Timer = 37000;
        }

        //Spell 3 timer
        if (Phase > 1)
            if (Spell_3_Timer.Expired(diff))
        {
            //Cast spell one on our current target.
            DoCast(m_creature->GetVictim(),SPELL_THREE);

            Spell_3_Timer = 19000;
        }

        //Beserk timer
        if (Phase > 1)
            if (Beserk_Timer.Expired(diff))
        {
            //Say our line then cast uber death spell
            DoPlaySoundToSet(m_creature,8588);
            DoYell(-1200190,LANG_UNIVERSAL,m_creature->GetVictim());
            DoCast(m_creature->GetVictim(),SPELL_BESERK);

            //Cast our beserk spell agian in 12 seconds if we didn't kill everyone
            Beserk_Timer = 12000;
        }

        //Phase timer
        if (Phase == 1)
            if (Phase_Timer.Expired(diff))
            {
                //Go to next phase
                Phase++;
                DoYell(-1200191,LANG_UNIVERSAL,NULL);
                DoCast(m_creature,SPELL_ENRAGE);
            }

        DoMeleeAttackIfReady();
    }
};

//This is the GetAI method used by all scripts that involve AI
//It is called every time a new creature using this script is created
CreatureAI* GetAI_custom_example(Creature *_Creature)
{
    return new custom_exampleAI (_Creature);
}

//This function is called when the player clicks an option on the gossip menu
void SendDefaultMenu_custom_example(Player *player, Creature *_Creature, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)               //Fight time
    {
        //Set our faction to hostile twoards all
        _Creature->setFaction(24);
        _Creature->Attack(player, true);
        player->PlayerTalkClass->CloseGossip();
    }
}

//This function is called when the player clicks an option on the gossip menu
bool GossipSelect_custom_example(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (sender == GOSSIP_SENDER_MAIN)
        SendDefaultMenu_custom_example(player, _Creature, action);

    return true;
}

//This function is called when the player opens the gossip menu
bool GossipHello_custom_example(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->PlayerTalkClass->SendGossipMenu(907,_Creature->GetGUID());

    return true;
}

//Our Recive emote function
bool ReceiveEmote_custom_example(Player *player, Creature *_Creature, uint32 emote)
{
    _Creature->HandleEmoteCommand(emote);

    if (emote == TEXTEMOTE_DANCE)
        ((custom_exampleAI*)_Creature->AI())->DoSay(-1200192,LANG_UNIVERSAL,NULL);

    if (emote == TEXTEMOTE_SALUTE)
        ((custom_exampleAI*)_Creature->AI())->DoSay(-1200193,LANG_UNIVERSAL,NULL);

    return true;
}

//This is the actual function called only once durring InitScripts()
//It must define all handled functions that are to be run in this script
//For example if you want this Script to handle Emotes you must include
//newscript->ReciveEmote = My_Emote_Function;
void AddSC_custom_example()
{
/*
    Script *newscript;

    newscript = new Script;
    newscript->Name="custom_example";
    newscript->GetAI = &GetAI_custom_example;
    newscript->pGossipHello = &GossipHello_custom_example;
    newscript->pGossipSelect = &GossipSelect_custom_example;
    newscript->pReceiveEmote = &ReceiveEmote_custom_example;
    newscript->RegisterSelf();
*/
}

