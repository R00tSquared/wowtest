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
SDName: Dark_Portal
SD%Complete: 90
SDComment: Still post-event needed and support for Time Keepers
SDCategory: Caverns of Time, The Dark Portal
EndScriptData */

/* ContentData
npc_medivh_bm
npc_time_rift
npc_saat
EndContentData */

#include "precompiled.h"
#include "def_dark_portal.h"

#define SAY_ENTER               -1269020        //intro speach by Medivh when entering instance
#define SAY_INTRO               -1269021
#define SAY_WEAK75              -1269022
#define SAY_WEAK50              -1269023
#define SAY_WEAK25              -1269024
#define SAY_DEATH               -1269025
#define SAY_WIN                 -1269026
#define SAY_ORCS_ENTER          -1269027
#define SAY_ORCS_ANSWER         -1269028

#define SPELL_CHANNEL           31556

#define SPELL_PORTAL_RUNE       32570                       //aura(portal on ground effect)
#define SPELL_CRYSTAL_ARCANE_EXPLOSION 32614
#define SPELL_BLACK_CRYSTAL     32563                       //aura
#define SPELL_PORTAL_CRYSTAL    32564                       //summon - not used

#define SPELL_BANISH_PURPLE     32566                       //aura
#define SPELL_BANISH_GREEN      32567                       //aura

#define SPELL_MANA_SHIELD       31635
#define SPELL_CORRUPT           31326
#define SPELL_CORRUPT_AEONUS    37853

#define C_COUNCIL_ENFORCER      17023

#define C_RKEEP 21104
#define C_RLORD 17839
#define C_ASSAS 17835
#define C_WHELP 21818
#define C_CHRON 17892
#define C_EXECU 18994
#define C_VANQU 18995

#define C_ORCS 17023
#define NPC_DARK_PORTAL_BLACK_CRYSTAL 18553
float OrcsLocationStart[6][4]=
{
   {-2098.4  , 7120.17 , 34.58 , 6.17},
   {-2097.82 , 7122.57 , 34.58 , 6.13 },
   {-2097.47 , 7124.29 , 34.58 , 6.14 },
   {-2096.87 , 7126.23 , 34.58 , 6.15},
   {-2096.45 , 7128.42 , 34.58 , 6.17},
   {-2095.79 , 7131.01 , 34.58 , 6.15 },
};

float OrcsLocationEnd[24][4]=
{
    {-2043.630005, 7114.538086, 23.423933, 6.247224},
    {-2043.460205, 7117.551758, 23.457970, 0.106981},
    {-2043.373779, 7120.466309, 23.518328, 6.215808},
    {-2043.122437, 7123.549316, 23.519186, 6.130200},
    {-2042.921021, 7126.144531, 23.610321, 0.034725},
    {-2042.728271, 7128.843262, 23.719294, 6.176540},

    {-2051.746582, 7114.506836, 26.108404, 6.215803},
    {-2051.626465, 7117.690918, 26.095806, 6.246433},
    {-2051.467773, 7120.655762, 26.169426, 6.215803},
    {-2051.380859, 7123.395508, 26.369158, 6.215803},
    {-2051.270020, 7126.339844, 26.722300, 6.215803},
    {-2050.597168, 7129.352051, 26.940187, 6.078356},

    {-2060.769287, 7113.901855, 29.096811, 6.282558},
    {-2060.588379, 7117.782715, 29.194443, 6.219728},
    {-2060.375977, 7121.002930, 29.268248, 6.219728},
    {-2060.272949, 7124.122559, 29.479456, 6.248003},
    {-2060.102295, 7127.264648, 29.851048, 6.248003},
    {-2059.472412, 7130.119629, 30.291830, 6.122341},

    {-2085.868652, 7117.961426, 34.587372, 6.134915},
    {-2085.583252, 7120.719727, 34.587372, 6.166329},
    {-2085.264404, 7123.415527, 34.587372, 6.162402},
    {-2084.791260, 7126.298828, 34.587372, 6.174182},
    {-2084.425537, 7129.454590, 34.583714, 6.178107},
    {-2084.150879, 7132.349609, 34.562080, 6.154544},
};

struct npc_medivh_bmAI : public ScriptedAI
{
    npc_medivh_bmAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = c->GetInstanceData();
        HeroicMode = c->GetMap()->IsHeroic();
        c->setActive(true);
    }

    ScriptedInstance *pInstance;
    SummonList summons;

    Timer SpellCorrupt_Timer;
    Timer DamageMelee_Timer;
    Timer Check_Timer;
    Timer Delay_Timer;
    Timer SummonCrystal_Timer;
    std::vector<uint64> crystals;
    uint32 CrystalsCount;

    bool Life75;
    bool Life50;
    bool Life25;

    bool HeroicMode;
    bool Intro;
    bool Delay;

    // Orcs after event
    Timer Orcs_Wave_Start_Timer;
    Timer Orcs_Wave_End_Timer;
    Timer Orcs_Wave_Clear_Timer;
    Timer Medivh_Speech_Timer;
    Timer Orc_General_Speech_Timer;
    int currentOrcWave;
    std::list<uint64> OrcsGUID;
    std::list<uint64>::iterator OrcIterator; // for waves
    uint64 OrcGeneral; // he will say text

    void Reset()
    {
        SpellCorrupt_Timer.Reset(0);
        DamageMelee_Timer.Reset(0);
        Delay_Timer.Reset(0);
        SummonCrystal_Timer.Reset(0);
        CrystalsCount = 0;

        Life75 = true;
        Life50 = true;
        Life25 = true;

        Intro = false;
        Delay = false;

        if (pInstance && (pInstance->GetData(TYPE_MEDIVH) == IN_PROGRESS))
        {
            me->CastSpell(me, SPELL_MANA_SHIELD, true);
            me->CastSpell(me, SPELL_CHANNEL, true);
        }
        else if (me->HasAura(SPELL_CHANNEL, 0))
        {
            me->RemoveAura(SPELL_MANA_SHIELD, 0);
            me->RemoveAura(SPELL_CHANNEL, 0);
        }

        Orcs_Wave_Start_Timer.Reset(0);
        Orcs_Wave_End_Timer.Reset(0);
        Orcs_Wave_Clear_Timer.Reset(0);
        Medivh_Speech_Timer.Reset(0);
        Orc_General_Speech_Timer.Reset(0);
        OrcsGUID.clear();
        OrcGeneral = 0;
        currentOrcWave = 0;
        summons.DespawnAll();
    }

    void JustSummoned(Creature* summon)
    {
        if(summon)
            summons.Summon(summon);

        if(summon->GetEntry() == NPC_DARK_PORTAL_BLACK_CRYSTAL)
            CrystalsCount++;
    }

    void MoveInLineOfSight(Unit *who)
    {
        //say enter phrase when in 50yd distance
        if (!Intro && pInstance->GetData(TYPE_MEDIVH) != DONE && who->GetTypeId() == TYPEID_PLAYER  && me->IsWithinDistInMap(who, 30.0f))
        {
            me->CastSpell(me, SPELL_PORTAL_RUNE, true);
            me->CastSpell(me, SPELL_CHANNEL, false);
            me->CastSpell(me, SPELL_MANA_SHIELD, true);
            DoScriptText(SAY_ENTER, me);
            Intro = true;
            Delay_Timer = 15000;
        }

        if (pInstance->GetData(TYPE_MEDIVH) != DONE && who->GetTypeId() == TYPEID_PLAYER  && !((Player*)who)->isGameMaster() && me->IsWithinDistInMap(who, 10.0f))
        {
            if (pInstance->GetData(TYPE_MEDIVH) == IN_PROGRESS)
                return;

            if(!Delay_Timer.GetInterval())
                DoScriptText(SAY_INTRO, me);
            else
                Delay = true;

            pInstance->SetData(TYPE_MEDIVH, IN_PROGRESS);
            Check_Timer = 5000;
            SummonCrystal_Timer = 4000;
        }
        else if (who->GetTypeId() == TYPEID_UNIT  && who->GetVictim() && who->GetVictim() == me && me->IsWithinDistInMap(who, 15.0f))
        {
            if (pInstance->GetData(TYPE_MEDIVH) != IN_PROGRESS)
                return;

            uint32 entry = who->GetEntry();
            if (entry == C_ASSAS || entry == C_WHELP || entry == C_CHRON || entry == C_EXECU || entry == C_VANQU)
            {
                who->StopMoving();
                who->CastSpell(me, SPELL_CORRUPT, false);
            }
            else if (entry == 20737 || entry == 17881)  //Aeonus
            {
                who->StopMoving();
                who->CastSpell(me,SPELL_CORRUPT_AEONUS,false);
            }
        }
    }

    void EnterCombat(Unit *who) {}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (SpellCorrupt_Timer.GetInterval())
            return;

        if (spell->Id == SPELL_CORRUPT_AEONUS)
            SpellCorrupt_Timer = 1000;

        if (spell->Id == SPELL_CORRUPT)
            SpellCorrupt_Timer = 3000;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by != me)
            damage = 0;

        if (DamageMelee_Timer.GetInterval() > 0)
            return;

        if (done_by->GetEntry() == C_RLORD || done_by->GetEntry() == C_RKEEP)
            DamageMelee_Timer = 5000;
        else
            DamageMelee_Timer = 1000;
    }

    void JustDied(Unit* Killer)
    {
        pInstance->SetData(TYPE_MEDIVH, FAIL);
        m_creature->SetRespawnTime(300);

        DoScriptText(SAY_DEATH, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Delay_Timer.Expired(diff))
        {
            if (Delay)
                DoScriptText(SAY_INTRO, me);
            Delay_Timer = 0;
        }

        if(SummonCrystal_Timer.Expired(diff) && (CrystalsCount < 3))
        {
            if (Creature* crystal = m_creature->SummonCreature(NPC_DARK_PORTAL_BLACK_CRYSTAL, -2016.9f, 7121.2f, 22.7f,3.14f, TEMPSUMMON_MANUAL_DESPAWN, 0))
                crystal->CastSpell(crystal, SPELL_BLACK_CRYSTAL, false);
            if (crystals.size() < 3)
                SummonCrystal_Timer = 7500;
            else
                SummonCrystal_Timer = 0;
        }
        if (SpellCorrupt_Timer.Expired(diff))
        {
            pInstance->SetData(TYPE_MEDIVH, SPECIAL);

            if (me->HasAura(SPELL_CORRUPT_AEONUS, 0))
                SpellCorrupt_Timer = 1000;
            else if (me->HasAura(SPELL_CORRUPT, 0))
                SpellCorrupt_Timer = 3000;
            else
                SpellCorrupt_Timer = 0;
        }

        if (DamageMelee_Timer.Expired(diff))
        {
            pInstance->SetData(TYPE_MEDIVH, SPECIAL);
            DamageMelee_Timer = 0;
        }

        if (Check_Timer.Expired(diff))
        {
            uint32 pct = pInstance->GetData(DATA_SHIELD);

            Check_Timer = 5000;

            if (Life25 && pct <= 25)
            {
                DoScriptText(SAY_WEAK25, me);
                Life25 = false;
                Check_Timer = 0;
                if (!crystals.empty())
                {
                    if (Creature* crystal = me->GetMap()->GetCreature(crystals[0]))
                    {
                        crystal->CastSpell(crystal, SPELL_CRYSTAL_ARCANE_EXPLOSION, false);
                        crystal->ForcedDespawn(3000);
                        crystals.erase(crystals.begin());
                    }
                }
            }
            else if (Life50 && pct <= 50)
            {
                DoScriptText(SAY_WEAK50, me);
                Life50 = false;
                if (!crystals.empty())
                {
                    if (Creature* crystal = me->GetMap()->GetCreature(crystals[0]))
                    {
                        crystal->CastSpell(crystal, SPELL_CRYSTAL_ARCANE_EXPLOSION, false);
                        crystal->ForcedDespawn(3000);
                        crystals.erase(crystals.begin());
                    }
                }
            }
            else if (Life75 && pct <= 75)
            {
                DoScriptText(SAY_WEAK75, me);
                Life75 = false;
                if (!crystals.empty())
                {
                    if (Creature* crystal = me->GetMap()->GetCreature(crystals[0]))
                    {
                        crystal->CastSpell(crystal, SPELL_CRYSTAL_ARCANE_EXPLOSION, false);
                        crystal->ForcedDespawn(3000);
                        crystals.erase(crystals.begin());
                    }
                }
            }

            //if we reach this it means event was running but at some point reset.
            if (pInstance->GetData(TYPE_MEDIVH) == NOT_STARTED)
            {
                me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                me->RemoveCorpse();
                me->Respawn();
                return;
            }

            if (pInstance->GetData(TYPE_MEDIVH) == DONE)
            {
                DoScriptText(SAY_WIN, me);
                Check_Timer = 0;
                Orcs_Wave_Start_Timer = 3000;
            }
        }

        if (Orcs_Wave_Start_Timer.Expired(diff))
        {
            for (int orc_counter = 0; orc_counter < 6; ++orc_counter)
            {
                Unit *orc = me->SummonCreature(C_ORCS, OrcsLocationStart[orc_counter][0],OrcsLocationStart[orc_counter][1],OrcsLocationStart[orc_counter][2],OrcsLocationStart[orc_counter][3], TEMPSUMMON_DEAD_DESPAWN,0);
                orc->MonsterMoveWithSpeed(OrcsLocationEnd[orc_counter + 6 * currentOrcWave][0], OrcsLocationEnd[orc_counter + 6 * currentOrcWave][1], OrcsLocationEnd[orc_counter + 6 * currentOrcWave][2], 500 + 1625 * (4 - currentOrcWave), true, true);
                orc->setFaction( me->getFaction() );
                OrcsGUID.push_back(orc->GetGUID());

                if (!OrcGeneral && orc_counter == 1 && currentOrcWave == 0)
                    OrcGeneral = orc->GetGUID();
            }

            if (currentOrcWave >= 3)
            {
                Orcs_Wave_Start_Timer = 0;
                Medivh_Speech_Timer = 9000;
                OrcsGUID.reverse();
                OrcIterator = OrcsGUID.begin();
            }
            else 
            {
                Orcs_Wave_Start_Timer = 2500;
                currentOrcWave++;
            }
        }

        if (Medivh_Speech_Timer.Expired(diff))
        {
            DoScriptText(SAY_ORCS_ENTER, me);
            Orc_General_Speech_Timer = 9000;
            Medivh_Speech_Timer = 0;
        }

        if (Orc_General_Speech_Timer.Expired(diff))
        {
            if (Unit* orc = me->GetCreature(OrcGeneral))
                DoScriptText(SAY_ORCS_ANSWER, orc);
            Orcs_Wave_End_Timer = 10000;
            Orc_General_Speech_Timer = 0;
        }

        if (Orcs_Wave_End_Timer.Expired(diff))
        {
            if (!OrcsGUID.empty() && OrcIterator != OrcsGUID.end())
            {
                int orc_counter = 5;
                while (orc_counter >= 0)
                {
                    if (Creature * orc = me->GetCreature(*OrcIterator++))
                    {
                        orc->MonsterMoveWithSpeed(OrcsLocationStart[orc_counter][0],OrcsLocationStart[orc_counter][1],OrcsLocationStart[orc_counter][2], 500 + 1625 * (4 - currentOrcWave), true, true);
                    }
                    orc_counter--;
                }
            }

            if (currentOrcWave < 0)
            {
                Orcs_Wave_End_Timer = 0;
                Orcs_Wave_Clear_Timer = 6500;
            }
            else 
            {
                Orcs_Wave_End_Timer = 500;
                currentOrcWave--;
            }
        }

        if (Orcs_Wave_Clear_Timer.Expired(diff))
        {
            if (!OrcsGUID.empty())
            {
                for (std::list<uint64>::iterator orcGUID = OrcsGUID.begin(); orcGUID != OrcsGUID.end(); ++orcGUID)
                {
                    if (Creature * orc = me->GetCreature(*orcGUID))
                    {
                        orc->SetVisibility(VISIBILITY_OFF);
                        orc->ForcedDespawn();
                    }
                }
            }

            OrcsGUID.clear();
            Orcs_Wave_Clear_Timer = 0;
        }
    }
};

CreatureAI* GetAI_npc_medivh_bm(Creature *_Creature)
{
    return new npc_medivh_bmAI (_Creature);
}

struct Wave
{
    uint32 PortalMob[4];                                    // spawns for portal waves (in order)
};

struct npc_time_riftAI : public ScriptedAI
{
    npc_time_riftAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = c->GetMap()->IsHeroic();
        c->setActive(true);
    }

    ScriptedInstance *pInstance;

    bool HeroicMode;

    Timer TimeRiftWave_Timer;
    uint8 RiftWaveCount;
    uint8 PortalCount;
    uint8 WaveId;

    void Reset()
    {

        TimeRiftWave_Timer.Reset(15000);
        RiftWaveCount = 0;

        PortalCount = pInstance->GetData(DATA_PORTAL_COUNT);

        if (PortalCount < 6)
            WaveId = 0;
        else if (PortalCount > 12)
            WaveId = 2;
        else
            WaveId = 1;
    }

    void EnterCombat(Unit *who) {}

    void JustDied(Unit* who)
    {
        me->RemoveCorpse();
    }

    void DoSummonAtRift(uint32 creature_entry)
    {
        if (!creature_entry)
            return;

        if (pInstance->GetData(TYPE_MEDIVH) != IN_PROGRESS)
        {
            me->InterruptNonMeleeSpells(true);
            me->RemoveAllAuras();
            return;
        }

        float x,y,z;
        me->GetRandomPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 10.0f, x, y, z);
        /*Unit* Summon = */me->SummonCreature(creature_entry, x, y, z, me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
    }

    void DoSelectSummon()
    {
        Wave PortalWaves[]=
        {
            C_ASSAS, C_WHELP, C_CHRON, 0,
            C_EXECU, C_CHRON, C_WHELP, C_ASSAS,
            C_EXECU, C_VANQU, C_CHRON, C_ASSAS
        };

        uint32 entry = 0;

        if ((RiftWaveCount > 2 && WaveId < 1) || RiftWaveCount > 3)
            RiftWaveCount = 0;

        entry = PortalWaves[WaveId].PortalMob[RiftWaveCount];
        debug_log("TSCR: npc_time_rift: summoning wave creature (Wave %u, Entry %u).",RiftWaveCount,entry);

        ++RiftWaveCount;

        if (entry == C_WHELP)
        {
            for (uint8 i = 0; i < 3; i++)
                DoSummonAtRift(entry);
        }
        else
            DoSummonAtRift(entry);
    }

    void UpdateAI(const uint32 diff)
    {
        PortalCount = pInstance->GetData(DATA_PORTAL_COUNT);

        if (TimeRiftWave_Timer.Expired(diff))
        {
            DoSelectSummon();

            if (PortalCount > 0 && PortalCount < 13)
                TimeRiftWave_Timer = urand(12000, 17000);
            else if (PortalCount > 12 && PortalCount < 18)
                TimeRiftWave_Timer = urand(7000, 12000);
            else
                TimeRiftWave_Timer = 0;

        }

        if (me->IsNonMeleeSpellCast(false))
            return;

        debug_log("TSCR: npc_time_rift: not casting anylonger, i need to die.");
        me->setDeathState(JUST_DIED);

        RiftWaveCount = 0;
        pInstance->SetData(TYPE_RIFT, SPECIAL);
    }
};

CreatureAI* GetAI_npc_time_rift(Creature *_Creature)
{
    return new npc_time_riftAI (_Creature);
}

struct rift_summonAI : public ScriptedAI
{
    rift_summonAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = c->GetMap()->IsHeroic();
        c->setActive(true);
    }

    ScriptedInstance *pInstance;

    bool HeroicMode;

    Timer Spell_Timer1;
    Timer Spell_Timer2;
    Timer Spell_Timer3;
    Timer Spell_Timer4;

    uint8 Type;
    bool aggro;
    bool frenzy;

    void Reset()
    {
        Unit* medivh = Unit::GetUnit(*me ,pInstance->GetData64(DATA_MEDIVH));

        me->setActive(true);
        me->SetNoCallAssistance(true);

        if (medivh && me->GetEntry() != C_RKEEP && me->GetEntry() != C_RLORD)
            AttackStart(medivh);

        Type = urand(0,1);

        switch (me->GetEntry())
        {
            case C_RKEEP:
            {
                if(Type) //mage
                {
                    Spell_Timer1.Reset(1000);                                    //Frostbolt
                    Spell_Timer2.Reset(HeroicMode ? 18500 : 12500);              //Pyroblast
                    Spell_Timer3.Reset(HeroicMode ? urand(12000, 27000) : 8000); //Blast Wave
                    Spell_Timer4.Reset(HeroicMode ? 15000 : 0);                  //Polymorph
                }
                else //warlock
                {
                    Spell_Timer1.Reset(7000);                            //Shadow Bolt Volley
                    Spell_Timer2.Reset(HeroicMode ? 6000 : 10000);       //Curse of Vulnerability
                    Spell_Timer3.Reset(urand(3000, 23000));              //Fear
                }
                frenzy = false;
                break;
            }
            case C_RLORD:
            {
                if(Type) //protection type
                {
                    Spell_Timer1.Reset(urand(6000, 12000));      //sunder armor
                    Spell_Timer2.Reset(urand(5000, HeroicMode ? 20000 : 25000));     //thunderclap
                }
                else //fury-arms
                {
                    Spell_Timer1.Reset(HeroicMode ? urand(6200, 18800) : urand(4800, 18800));    //knockdown
                    Spell_Timer2.Reset(HeroicMode ? urand(4900, 17700) : urand(6100, 18000));    //mortal strike
                    Spell_Timer3.Reset(HeroicMode ? urand(4600, 15700) : urand(7200, 11800));    //harmstring
                }
                break;
            }
            case C_ASSAS:
            {
                if(Type)    //combat
                {
                    Spell_Timer1.Reset(HeroicMode ? urand(500, 7300) : urand(1200, 11100));      //sinister strike
                    Spell_Timer2.Reset(HeroicMode ? urand(1000, 15800) : urand(1900, 10100));    //rupture
                    Spell_Timer3.Reset(HeroicMode ? urand(800, 7800) : 0);                       //crippling poison
                }
                else        //assasin
                {
                    Spell_Timer1.Reset(urand(1200, 12400));                      //kidney shot
                    Spell_Timer2.Reset(HeroicMode ? urand(1000, 6500) : 0);      //deadly poison
                    Spell_Timer3.Reset(urand(4800, 7200));                       //backstab
                }
                break;
            }
            case C_WHELP:
                break;
            case C_CHRON:
            {
                if(Type) //frost
                {
                    Spell_Timer1.Reset(HeroicMode? urand(8000, 10000) : urand(8000, 16000));     //frostbolt
                    Spell_Timer2.Reset(HeroicMode ? urand(3600, 12200) : urand(3700, 12900));    //frost nova
                }
                else //arcane
                {
                    Spell_Timer1.Reset(HeroicMode ? urand(1200, 3400) : urand(2900, 5400)); //arcane bolt
                    Spell_Timer2.Reset(urand(8600, 18500));                                  //arcane explosion
                }
                break;
            }
            case C_EXECU:
                Spell_Timer1.Reset(HeroicMode ? urand(2000, 11700) : urand(7300, 14000));    //cleave
                Spell_Timer2.Reset(HeroicMode ? urand(2000, 3900) : 7200);                   //strike
                Spell_Timer3.Reset(HeroicMode ? urand(600, 10200) : 0);                      //harmstring
                break;
            case C_VANQU:
                Spell_Timer1.Reset(1000);                //scorch + shadow bolt
                Spell_Timer2.Reset(urand(5900, 6000));   //fire blast
                break;
            default:
                break;
        }

    }

    void EnterCombat(Unit *who)
    {
        if (who->GetTypeId() == TYPEID_UNIT)
            aggro = false;

        if (/*who->GetTypeId() == TYPEID_UNIT  && */me->GetEntry() != C_WHELP && me->GetEntry() != C_VANQU)
        {
            if (urand(0,100) > 30)   //30% chance on yell
            {
                switch (rand()%9)
                {
                  case 0: me->MonsterYell(-1200342, 0, me->GetGUID()); break;
                  case 1: me->MonsterYell(-1200343, 0, me->GetGUID()); break;
                  case 2: me->MonsterYell(-1200344, 0, me->GetGUID()); break;
                  case 3: me->MonsterYell(-1200345, 0, me->GetGUID()); break;
                  case 4: me->MonsterYell(-1200346, 0, me->GetGUID()); break;
                  case 5: me->MonsterYell(-1200347, 0, me->GetGUID()); break;
                  case 6: me->MonsterYell(-1200348, 0, me->GetGUID()); break;
                  case 7: me->MonsterYell(-1200349, 0, me->GetGUID()); break;
                  case 8: me->MonsterYell(-1200350, 0, me->GetGUID()); break;
                }
            }
        }
        else if (me->GetEntry() == C_VANQU)
        {
            if (urand(0,100) > 30)   //30% chance on say
            {
                switch (rand()%2)
                {
                  case 0: me->MonsterYell(-1200351, 0, me->GetGUID()); break;
                  case 1: me->MonsterYell(-1200352, 0, me->GetGUID()); break;
                }
            }
        }
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (!aggro && done_by->GetTypeId() == TYPEID_PLAYER)
        {
            AttackStart(done_by);
            aggro = true;
        }
    }

    void JustDied(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (me->GetVictim() && me->GetVictim()->GetTypeId() == TYPEID_PLAYER)
        {
            switch (me->GetEntry())
            {
                case C_RKEEP:
                {
                    if (Type) //mage
                    {
                        if (Spell_Timer1.Expired(diff)) //frostbolt
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?38534:36279);
                            Spell_Timer1 = urand(8000, HeroicMode ? 10000 : 16000);
                        }

                        if (Spell_Timer2.Expired(diff)) //pyroblast
                        {
                            Spell_Timer1 = 8000;

                            Unit* target = SelectUnit(SELECT_TARGET_NEAREST, 0, 70, true, me->getVictimGUID());
                            if (!target)
                                target = me->GetVictim();

                            if (target)
                                AddSpellToCast(target, HeroicMode?38535:36277);

                            Spell_Timer2 = HeroicMode ? urand(14000, 24000) : urand(12000, 17000);
                        }

                        if (Spell_Timer3.Expired(diff)) //blast wave
                        {
                            ForceSpellCast(me, HeroicMode?38536:36278, DONT_INTERRUPT, true);
                            Spell_Timer3 = HeroicMode ? urand(15000, 25000) : 13000;
                        }

                        if (HeroicMode && Spell_Timer4.Expired(diff)) //polymorph
                        {
                            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 70, true, me->getVictimGUID());
                            if (target)
                                AddSpellToCast(target, 13323);
                            Spell_Timer4 = 30000;
                        }
                    }
                    else //warlock
                    {
                        if (Spell_Timer1.Expired(diff)) //shadow bolt volley
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?38533:36275);
                            Spell_Timer1 = HeroicMode ? Spell_Timer2.GetTimeLeft() + 1500 : urand(10000, 25000);
                        }

                        if (Spell_Timer2.Expired(diff)) //curse of vulnerability
                        {
                            Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,70,true);
                            if (target)
                                AddSpellToCast(target, 36276, true);
                            Spell_Timer2 = HeroicMode ? urand(9000, 14000) : Spell_Timer1.GetTimeLeft() + 2000;
                        }

                        if (Spell_Timer3.Expired(diff)) //fear
                        {
                            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 70, true);
                            if (target)
                                AddSpellToCast(target, 12542);
                            Spell_Timer3 = urand(15000, 25000);
                        }

                        if (!frenzy && me->GetHealthPercent() < 30)
                        {
                            AddSpellToCast(me, 8269, true);
                            frenzy = true;
                        }
                    }
                    break;
                }
                case C_RLORD:
                {
                    if(Type)    //protection type
                    {
                        if (Spell_Timer1.Expired(diff)) //sunder armor
                        {
                            AddSpellToCast(me->GetVictim(), 16145, true);
                            Spell_Timer1 = urand(6000, 9000);
                        }

                        if (Spell_Timer2.Expired(diff)) //thunderclap
                        {
                            AddSpellToCast(me, HeroicMode?38537:36214, true);
                            Spell_Timer2 = HeroicMode ? urand(12000, 17000) : urand(10000, 25000);
                        }
                    }
                    else //fury-arms
                    {
                        
                        if (Spell_Timer1.Expired(diff)) //knockback
                        {
                            AddSpellToCast(me->GetVictim(), 11428, true);
                            Spell_Timer1 = HeroicMode ? urand(13300, 19100) : urand(18100, 38500);
                        }

                        if (Spell_Timer2.Expired(diff)) //mortal strike
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?35054:15708, true);
                            Spell_Timer2 = HeroicMode ? urand(10300, 14500) : urand(10800, 15800);
                        }

                        if (Spell_Timer3.Expired(diff)) //harmstring
                        {
                            AddSpellToCast(me->GetVictim(), 9080, true);
                            Spell_Timer3 = HeroicMode ? urand(11600, 18100) : urand(15500, 26500);
                        }
                    }

                    if (!frenzy && me->GetHealthPercent() < 30)
                    {
                        AddSpellToCast(me, 8269, true);
                        frenzy = true;
                    }
                    break;
                }
                case C_ASSAS:
                {
                    if (Type) //combat
                    {
                        if (Spell_Timer1.Expired(diff)) //sinister strike
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?15667:14873, true);
                            Spell_Timer1 = HeroicMode ? urand(3500, 14500) : urand(4500, 15300);
                        }

                        if (Spell_Timer2.Expired(diff)) //rupture
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?15583:14874, true);
                            Spell_Timer2 = HeroicMode ? urand(10100, 20500) : urand(10400, 21600);
                        }

                        if (Spell_Timer3.Expired(diff)) //crippling poison
                        {
                            AddSpellToCast(me->GetVictim(), 9080, true);
                            Spell_Timer3 = HeroicMode ? urand(12200, 62800) : 0;
                        }
                    }
                    else //assasin
                    {
                        if (Spell_Timer1.Expired(diff)) //kidney shot
                        {
                            AddSpellToCast(me->GetVictim(), 30832, true);
                            Spell_Timer1 = urand(20100, 24900);
                        }

                        if (Spell_Timer2.Expired(diff)) //deadly poison
                        {
                            AddSpellToCast(me->GetVictim(), 38520, true);
                            Spell_Timer2 = HeroicMode ? urand(12300, 24200) : 0;
                        }

                        if (Spell_Timer3.Expired(diff) && me->GetVictim() && !me->GetVictim()->HasInArc(M_PI, me)) //backstab
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?15657:7159, true);
                            Spell_Timer3 = urand(4800, 7200);
                        }
                    }
                    break;
                }
                case C_WHELP:
                    break;
                case C_CHRON:
                {
                    if (me->GetPower(POWER_MANA)*100/me->GetMaxPower(POWER_MANA) > 15)
                    {
                        if (Type) //frost
                        {
                            if (Spell_Timer1.Expired(diff)) //frostbolt
                            {
                                AddSpellToCast(me->GetVictim(), HeroicMode?12675:15497);
                                Spell_Timer1 = urand(2900, 5400);
                            }

                            if (Spell_Timer2.Expired(diff) && me->IsWithinCombatRange(me->GetVictim(), 10)) //frost nova
                            {
                                AddSpellToCast(me, HeroicMode?15531:15063, true);
                                Spell_Timer2.Reset(HeroicMode ? urand(22200, 25700) : urand(33800, 39800));
                            }
                        }
                        else //arcane
                        {
                            if (Spell_Timer1.Expired(diff)) //arcane bolt
                            {
                                AddSpellToCast(me->GetVictim(), HeroicMode?15230:15124);
                                Spell_Timer1 = HeroicMode ? urand(1200, 3400) : urand(2900, 5400);
                            }

                            if (Spell_Timer2.Expired(diff) && me->IsWithinCombatRange(me->GetVictim(), 10)) //arcane explosion
                            {
                                AddSpellToCast(me, HeroicMode?33623:33860, true);
                                Spell_Timer2.Reset(HeroicMode ? urand(8000, 10100) : urand(9500, 10100));
                            }
                        }
                    }
                    break;
                }
                case C_EXECU:
                {
                    if (Spell_Timer1.Expired(diff)) //cleave
                    {
                        AddSpellToCast(me->GetVictim(), 15496, true);
                        Spell_Timer1 = HeroicMode ? urand(6000, 11700) : urand(7300, 14000);
                    }

                    if (Spell_Timer2.Expired(diff)) //strike
                    {
                        AddSpellToCast(me->GetVictim(), HeroicMode?34920:15580, true);
                        Spell_Timer2 = HeroicMode ? urand(3900, 9700) : urand(9700, 20300);
                    }

                    if (Spell_Timer3.Expired(diff)) //harmstring
                    {
                        AddSpellToCast(me->GetVictim(), 9080, true);
                        Spell_Timer3 = HeroicMode ? urand(10800, 15800) : 0;
                    }
                    break;
                }
                case C_VANQU:
                {
                    if (me->GetPower(POWER_MANA)*100/me->GetMaxPower(POWER_MANA) > 15)
                    {
                        if (Spell_Timer1.Expired(diff)) //scorch + shadow bolt
                        {
                            bool fire = urand(0,1);
                            if (fire)
                                AddSpellToCast(me->GetVictim(), HeroicMode?36807:15241);
                            else
                                AddSpellToCast(me->GetVictim(), HeroicMode?15472:12739);
                            Spell_Timer1 = urand(3500, 4500);
                        }

                        if (Spell_Timer2.Expired(diff)) //fire blast
                        {
                            AddSpellToCast(me->GetVictim(), HeroicMode?38526:13341, true);
                            Spell_Timer2 = urand(5900, 6000);
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            CastNextSpellIfAnyAndReady();
            DoMeleeAttackIfReady();
        }

        if (pInstance->GetData(TYPE_MEDIVH) == FAIL)
        {
            me->Kill(me, false);
            me->RemoveCorpse();
        }
    }
};

CreatureAI* GetAI_rift_summon(Creature *_Creature)
{
    return new rift_summonAI (_Creature);
}

#define SAY_SAAT_WELCOME        -1269019

#define GOSSIP_ITEM_OBTAIN      16285
#define SPELL_CHRONO_BEACON     34975
#define ITEM_CHRONO_BEACON      24289

bool GossipHello_npc_saat(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if (player->GetQuestStatus(QUEST_OPENING_PORTAL) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(ITEM_CHRONO_BEACON,1))
    {
        player->ADD_GOSSIP_ITEM(0,player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OBTAIN),GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(10000,_Creature->GetGUID());
        return true;
    }
    else if (player->GetQuestRewardStatus(QUEST_OPENING_PORTAL) && !player->HasItemCount(ITEM_CHRONO_BEACON,1))
    {
        player->ADD_GOSSIP_ITEM(0,player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OBTAIN),GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(10001,_Creature->GetGUID());
        return true;
    }

    player->SEND_GOSSIP_MENU(10002,_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_saat(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player,SPELL_CHRONO_BEACON,false);
    }
    return true;
}

struct mob_dark_crystalAI : public ScriptedAI
{
    mob_dark_crystalAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    bool PointReached;
    ScriptedInstance* pInstance;
    float x, y, r, c, mx, my;

    void InitializeAI()
    {
        me->SetIgnoreVictimSelection(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        me->SetSelection(0);
        me->SetLevitate(true);
        PointReached = true;
        mx = (-2023.58f);
        my = (7121.7f);
        c = (3.14f * 0.75f);
        r = 8.5f;
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetSelection())
            me->SetSelection(0);

        if (PointReached)
        {
            y = my + r * cos(c);
            x = mx + r * sin(c);
            PointReached = false;
            m_creature->GetMotionMaster()->MovePoint(1, x, y, 23);
            c += M_PI / 50;

            if (c > 2 * M_PI)
                c -= 2 * M_PI;
            if (c < 0)
                c += 2 * M_PI;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        PointReached = true;
    }

};

CreatureAI* GetAI_mob_dark_crystal(Creature *_Creature)
{
    return new mob_dark_crystalAI (_Creature);
}

struct npc_17918AI : public ScriptedAI
{
    npc_17918AI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        DespawnTimer.Reset(85000);
    }

    Timer DespawnTimer;
    Timer BreathTimer;

    void Reset()
    {
        BreathTimer.Reset(urand(2000, 3000));
        me->Say(-1200353, LANG_UNIVERSAL, 0);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(who->GetTypeId() == TYPEID_UNIT && me->IsWithinDistInMap(who, 25.0f))
        {
            if(who->GetEntry() == C_RKEEP || who->GetEntry() == C_RLORD || who->GetEntry() == C_ASSAS || 
                who->GetEntry() == C_WHELP || who->GetEntry() == C_CHRON || who->GetEntry() == C_EXECU || who->GetEntry() == C_VANQU)
                me->AI()->AttackStart(who);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (DespawnTimer.Expired(diff))
        {
            me->DisappearAndDie();
            DespawnTimer = 0;
            return;
        }

        if (!UpdateVictim())
            return;

        if (BreathTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 31478, false);
            BreathTimer = urand(8000, 12000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_17918(Creature* pCreature)
{
    return new npc_17918AI(pCreature);
}


void AddSC_dark_portal()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_medivh_bm";
    newscript->GetAI = &GetAI_npc_medivh_bm;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_time_rift";
    newscript->GetAI = &GetAI_npc_time_rift;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "rift_summon";
    newscript->GetAI = &GetAI_rift_summon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_saat";
    newscript->pGossipHello = &GossipHello_npc_saat;
    newscript->pGossipSelect = &GossipSelect_npc_saat;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dark_crystal";
    newscript->GetAI = &GetAI_mob_dark_crystal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_17918";
    newscript->GetAI = &GetAI_npc_17918;
    newscript->RegisterSelf();
}
