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
SDName: Boss_Chromaggus
SD%Complete: 99
SDComment: Debugging
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define EMOTE_FRENZY                -1469002
#define EMOTE_SHIMMER               -1469003

//These spells are actually called elemental shield
//What they do is decrease all damage by 75% then they increase
//One school of damage by 1100%
#define SPELL_FIRE_VURNALBILTY      22277
#define SPELL_FROST_VURNALBILTY     22278
#define SPELL_SHADOW_VURNALBILTY    22279
#define SPELL_NATURE_VURNALBILTY    22280
#define SPELL_ARCANE_VURNALBILTY    22281

#define SPELL_INCINERATE            23308                   //Incinerate 23308,23309
#define SPELL_TIMELAPSE             23310                   //Time lapse 23310, 23311(old threat mod that was removed in 2.01)
#define SPELL_CORROSIVEACID         23313                   //Corrosive Acid 23313, 23314
#define SPELL_IGNITEFLESH           23315                   //Ignite Flesh 23315,23316
#define SPELL_FROSTBURN             23187                   //Frost burn 23187, 23189

//Brood Affliction 23173 - Scripted Spell that cycles through all targets within 100 yards and has a chance to cast one of the afflictions on them
//Since Scripted spells arn't coded I'll just write a function that does the same thing
#define SPELL_BROODAF_BLUE          23153                   //Blue affliction 23153
#define SPELL_BROODAF_BLACK         23154                   //Black affliction 23154
#define SPELL_BROODAF_RED           23155                   //Red affliction 23155 (23168 on death)
#define SPELL_BROODAF_BRONZE        23170                   //Bronze Affliction  23170
#define SPELL_BROODAF_GREEN         23169                   //Brood Affliction Green 23169

#define SPELL_CHROMATIC_MUT_1       23174                   //Spell cast on player if they get all 5 debuffs

#define SPELL_FRENZY                28371                   //The frenzy spell may be wrong
#define SPELL_ENRAGE                28747

struct boss_chromaggusAI : public ScriptedAI
{
    boss_chromaggusAI(Creature *c) : ScriptedAI(c)
    {
        //Select the 2 breaths that we are going to use until despawned
        //5 possiblities for the first breath, 4 for the second, 20 total possiblites
        //This way we don't end up casting 2 of the same breath
        //TL TL would be stupid
        srand(time(NULL));
        switch (rand()%20)
        {
            //B1 - Incin
            case 0:
                Breath1_Spell = SPELL_INCINERATE;
                Breath2_Spell = SPELL_TIMELAPSE;
                break;
            case 1:
                Breath1_Spell = SPELL_INCINERATE;
                Breath2_Spell = SPELL_CORROSIVEACID;
                break;
            case 2:
                Breath1_Spell = SPELL_INCINERATE;
                Breath2_Spell = SPELL_IGNITEFLESH;
                break;
            case 3:
                Breath1_Spell = SPELL_INCINERATE;
                Breath2_Spell = SPELL_FROSTBURN;
                break;

                //B1 - TL
            case 4:
                Breath1_Spell = SPELL_TIMELAPSE;
                Breath2_Spell = SPELL_INCINERATE;
                break;
            case 5:
                Breath1_Spell = SPELL_TIMELAPSE;
                Breath2_Spell = SPELL_CORROSIVEACID;
                break;
            case 6:
                Breath1_Spell = SPELL_TIMELAPSE;
                Breath2_Spell = SPELL_IGNITEFLESH;
                break;
            case 7:
                Breath1_Spell = SPELL_TIMELAPSE;
                Breath2_Spell = SPELL_FROSTBURN;
                break;

                //B1 - Acid
            case 8:
                Breath1_Spell = SPELL_CORROSIVEACID;
                Breath2_Spell = SPELL_INCINERATE;
                break;
            case 9:
                Breath1_Spell = SPELL_CORROSIVEACID;
                Breath2_Spell = SPELL_TIMELAPSE;
                break;
            case 10:
                Breath1_Spell = SPELL_CORROSIVEACID;
                Breath2_Spell = SPELL_IGNITEFLESH;
                break;
            case 11:
                Breath1_Spell = SPELL_CORROSIVEACID;
                Breath2_Spell = SPELL_FROSTBURN;
                break;

                //B1 - Ignite
            case 12:
                Breath1_Spell = SPELL_IGNITEFLESH;
                Breath2_Spell = SPELL_INCINERATE;
                break;
            case 13:
                Breath1_Spell = SPELL_IGNITEFLESH;
                Breath2_Spell = SPELL_CORROSIVEACID;
                break;
            case 14:
                Breath1_Spell = SPELL_IGNITEFLESH;
                Breath2_Spell = SPELL_TIMELAPSE;
                break;
            case 15:
                Breath1_Spell = SPELL_IGNITEFLESH;
                Breath2_Spell = SPELL_FROSTBURN;
                break;

                //B1 - Frost
            case 16:
                Breath1_Spell = SPELL_FROSTBURN;
                Breath2_Spell = SPELL_INCINERATE;
                break;
            case 17:
                Breath1_Spell = SPELL_FROSTBURN;
                Breath2_Spell = SPELL_TIMELAPSE;
                break;
            case 18:
                Breath1_Spell = SPELL_FROSTBURN;
                Breath2_Spell = SPELL_CORROSIVEACID;
                break;
            case 19:
                Breath1_Spell = SPELL_FROSTBURN;
                Breath2_Spell = SPELL_IGNITEFLESH;
                break;
        };

        pInstance = (ScriptedInstance*)c->GetInstanceData();

        EnterEvadeMode();
    }

    ScriptedInstance * pInstance;
    uint32 Breath1_Spell;
    uint32 Breath2_Spell;
    uint32 CurrentVurln_Spell;

    Timer _ChangedNameShimmer_Timer;
    Timer _ChangedNameBreath1_Timer;
    Timer _ChangedNameBreath2_Timer;
    Timer _ChangedNameAffliction_Timer;
    Timer _ChangedNameFrenzy_Timer;
    bool Enraged;

    void Reset()
    {
        CurrentVurln_Spell = 0;                             //We use this to store our last vurlnability spell so we can remove it later

        _ChangedNameShimmer_Timer.Reset(1);                                //Time till we change vurlnerabilites
        _ChangedNameBreath1_Timer.Reset(30000);                              //First breath is 30 seconds
        _ChangedNameBreath2_Timer.Reset(60000);                              //Second is 1 minute so that we can alternate
        _ChangedNameAffliction_Timer.Reset(10000);                           //This is special - 5 seconds means that we cast this on 1 player every 5 sconds
        _ChangedNameFrenzy_Timer.Reset(15000);

        Enraged = false;

        if (pInstance && pInstance->GetData(DATA_CHROMAGGUS_EVENT) != DONE)
            pInstance->SetData(DATA_CHROMAGGUS_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_CHROMAGGUS_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_CHROMAGGUS_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        if (_ChangedNameShimmer_Timer.Expired(diff))
        {
            //Remove old vurlnability spell
            if (CurrentVurln_Spell)
                m_creature->RemoveAurasDueToSpell(CurrentVurln_Spell);

            //Cast new random vurlnabilty on self
            uint32 spell;

            spell = RAND(SPELL_FIRE_VURNALBILTY, SPELL_FROST_VURNALBILTY, SPELL_SHADOW_VURNALBILTY, SPELL_NATURE_VURNALBILTY, SPELL_ARCANE_VURNALBILTY);

            DoCast(m_creature,spell);
            CurrentVurln_Spell = spell;

            DoScriptText(EMOTE_SHIMMER, m_creature);
            _ChangedNameShimmer_Timer = 45000;
        }
        

        if (_ChangedNameBreath1_Timer.Expired(diff))
        {
            if (Unit* target = m_creature->GetVictim())
            {
                DoCast(target,Breath1_Spell);
                _ChangedNameBreath1_Timer = 60000;
            }
        }
        

        if (_ChangedNameBreath2_Timer.Expired(diff))
        {
            if (Unit* target = m_creature->GetVictim())
            {
                DoCast(m_creature->GetVictim(),Breath2_Spell);
                _ChangedNameBreath2_Timer = 60000;
            }
        }
        

        if (_ChangedNameAffliction_Timer.Expired(diff))
        {
            uint32 SpellAfflict = 0;

            SpellAfflict = HeroicMode ? RAND(SPELL_BROODAF_BLUE, SPELL_BROODAF_BLACK, SPELL_BROODAF_RED, SPELL_BROODAF_GREEN) : RAND(SPELL_BROODAF_BLUE, SPELL_BROODAF_BLACK, SPELL_BROODAF_RED, SPELL_BROODAF_BRONZE, SPELL_BROODAF_GREEN);

            std::list<HostileReference*>::iterator i;

            for (i = m_creature->getThreatManager().getThreatList().begin();i != m_creature->getThreatManager().getThreatList().end();)
            {
                Unit* pUnit = NULL;
                if ((*i) == nullptr || !(*i) || !((*i)->getSource()))
                {
                    sLog.outLog(LOG_DEFAULT, "Crash on Chromaggus AI no *i!");
                    break;
                }
                pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
                ++i;

                if (pUnit)
                {
                    //Cast affliction
                    DoCast(pUnit, SpellAfflict, true);

                    //Chromatic mutation if target is effected by all afflictions
                    if (pUnit->HasAura(SPELL_BROODAF_BLUE,0)
                        && pUnit->HasAura(SPELL_BROODAF_BLACK,0)
                        && pUnit->HasAura(SPELL_BROODAF_RED,0)
                        && pUnit->HasAura(SPELL_BROODAF_BRONZE,0)
                        && pUnit->HasAura(SPELL_BROODAF_GREEN,0))
                    {
                        pUnit->RemoveAllAuras();
                        DoCast(pUnit,SPELL_CHROMATIC_MUT_1);

                        //Chromatic mutation is causing issues
                        //Assuming it is caused by a lack of core support for Charm
                        //So instead we instant kill our target
                    }
                }
            }

            _ChangedNameAffliction_Timer = 10000;
        }
        

        if (_ChangedNameFrenzy_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_FRENZY);
            DoScriptText(EMOTE_FRENZY, m_creature);
            _ChangedNameFrenzy_Timer = 10000 + (rand() % 5000);
        }
        
           

        //Enrage if not already enraged and below 20%
        if (!Enraged && (me->GetHealthPercent()) < 20)
        {
            DoCast(m_creature,SPELL_ENRAGE);
            Enraged = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_chromaggus(Creature *_Creature)
{
    return new boss_chromaggusAI (_Creature);
}

void AddSC_boss_chromaggus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_chromaggus";
    newscript->GetAI = &GetAI_boss_chromaggus;
    newscript->RegisterSelf();
}


