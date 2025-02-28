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
SDName: Boss_Tomb_Of_Seven
SD%Complete: 95
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "def_blackrock_depths.h"

#define FACTION_NEUTRAL             734
#define FACTION_HOSTILE             754

#define SPELL_SUNDERARMOR           24317
#define SPELL_SHIELDBLOCK           12169
#define SPELL_STRIKE                15580

struct boss_angerrelAI : public ScriptedAI
{
    boss_angerrelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameSunderArmor_Timer;
    Timer _ChangedNameShieldBlock_Timer;
    Timer _ChangedNameStrike_Timer;

    void Reset()
    {
        _ChangedNameSunderArmor_Timer.Reset(8000);
        _ChangedNameShieldBlock_Timer.Reset(15000);
        _ChangedNameStrike_Timer.Reset(12000);

        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameSunderArmor_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SUNDERARMOR);
            _ChangedNameSunderArmor_Timer = 28000;
        }


        if (_ChangedNameShieldBlock_Timer.Expired(diff))
        {
            DoCast(me, SPELL_SHIELDBLOCK);
            _ChangedNameShieldBlock_Timer = 25000;
        }


        if (_ChangedNameStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_STRIKE);
            _ChangedNameStrike_Timer = 10000;
        }


        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_angerrel(Creature *creature)
{
    return new boss_angerrelAI(creature);
}

#define SPELL_SINISTERSTRIKE        15581
#define SPELL_BACKSTAB              15582
#define SPELL_GOUGE                 13579

struct boss_doperelAI : public ScriptedAI
{
    boss_doperelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameSinisterStrike_Timer;
    Timer _ChangedNameBackStab_Timer;
    Timer _ChangedNameGouge_Timer;

    void Reset()
    {
        _ChangedNameSinisterStrike_Timer.Reset(8000);
        _ChangedNameBackStab_Timer.Reset(12000);
        _ChangedNameGouge_Timer.Reset(6000);

        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameSinisterStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SINISTERSTRIKE);
            _ChangedNameSinisterStrike_Timer = 7000;
        }



        if (_ChangedNameBackStab_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_BACKSTAB);
            _ChangedNameBackStab_Timer = 6000;
        }



        if (_ChangedNameGouge_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_GOUGE);
            _ChangedNameGouge_Timer = 8000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_doperel(Creature *creature)
{
    return new boss_doperelAI(creature);
}

#define SPELL_SHADOWBOLT        17483                       //Not sure if right ID
#define SPELL_MANABURN          10876
#define SPELL_SHADOWSHIELD      22417

struct boss_haterelAI : public ScriptedAI
{
    boss_haterelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameShadowBolt_Timer;
    Timer _ChangedNameManaBurn_Timer;
    Timer _ChangedNameShadowShield_Timer;
    Timer _ChangedNameStrike_Timer2;

    void Reset()
    {
        _ChangedNameShadowBolt_Timer.Reset(15000);
        _ChangedNameManaBurn_Timer.Reset(3000);
        _ChangedNameShadowShield_Timer.Reset(8000);
        _ChangedNameStrike_Timer2.Reset(12000);

        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameShadowBolt_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if (target) DoCast(target, SPELL_SHADOWBOLT);
            _ChangedNameShadowBolt_Timer = 7000;
        }
        

        if (_ChangedNameManaBurn_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(target, SPELL_MANABURN);

            _ChangedNameManaBurn_Timer = 13000;
        }
        

        if (_ChangedNameShadowShield_Timer.Expired(diff))
        {
            DoCast(me, SPELL_SHADOWSHIELD);
            _ChangedNameShadowShield_Timer = 25000;
        }
        

        if (_ChangedNameStrike_Timer2.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_STRIKE);
            _ChangedNameStrike_Timer2 = 10000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_haterel(Creature *creature)
{
    return new boss_haterelAI(creature);
}

#define SPELL_MINDBLAST             15587
#define SPELL_HEAL                  15586
#define SPELL_PRAYEROFHEALING       15585
#define SPELL_SHIELD                10901

struct boss_vilerelAI : public ScriptedAI
{
    boss_vilerelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameMindBlast_Timer;
    Timer _ChangedNameHeal_Timer;
    Timer _ChangedNamePrayerOfHealing_Timer;
    Timer _ChangedNameShield_Timer;

    void Reset()
    {
        _ChangedNameMindBlast_Timer.Reset(10000);
        _ChangedNameHeal_Timer.Reset(35000);
        _ChangedNamePrayerOfHealing_Timer.Reset(25000);
        _ChangedNameShield_Timer.Reset(3000);

        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameMindBlast_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_MINDBLAST);
            _ChangedNameMindBlast_Timer = 7000;
        }
        

        if (_ChangedNameHeal_Timer.Expired(diff))
        {
            DoCast(me, SPELL_HEAL);
            _ChangedNameHeal_Timer = 20000;
        }
        

        if (_ChangedNamePrayerOfHealing_Timer.Expired(diff))
        {
            DoCast(me, SPELL_PRAYEROFHEALING);
            _ChangedNamePrayerOfHealing_Timer = 30000;
        }
        

        if (_ChangedNameShield_Timer.Expired(diff))
        {
            DoCast(me, SPELL_SHIELD);
            _ChangedNameShield_Timer = 30000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_vilerel(Creature *creature)
{
    return new boss_vilerelAI(creature);
}

#define SPELL_FROSTBOLT         16799
#define SPELL_FROSTARMOR        15784                       //This is actually a buff he gives himself
#define SPELL_BLIZZARD          19099
#define SPELL_FROSTNOVA         15063
#define SPELL_FROSTWARD         15004

struct boss_seethrelAI : public ScriptedAI
{
    boss_seethrelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameFrostArmor_Timer;
    Timer _ChangedNameFrostbolt_Timer;
    Timer _ChangedNameBlizzard_Timer;
    Timer _ChangedNameFrostNova_Timer;
    Timer _ChangedNameFrostWard_Timer;

    void Reset()
    {
        _ChangedNameFrostArmor_Timer.Reset(2000);
        _ChangedNameFrostbolt_Timer.Reset(6000);
        _ChangedNameBlizzard_Timer.Reset(18000);
        _ChangedNameFrostNova_Timer.Reset(12000);
        _ChangedNameFrostWard_Timer.Reset(25000);

        me->CastSpell(me, SPELL_FROSTARMOR, true);
        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameFrostArmor_Timer.Expired(diff))
        {
            DoCast(me, SPELL_FROSTARMOR);
            _ChangedNameFrostArmor_Timer = 180000;
        }
        

        if (_ChangedNameFrostbolt_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_FROSTBOLT);
            _ChangedNameFrostbolt_Timer = 15000;
        }
        

        if (_ChangedNameBlizzard_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(target, SPELL_BLIZZARD);

            _ChangedNameBlizzard_Timer = 22000;
        }
        
           

        if (_ChangedNameFrostNova_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_FROSTNOVA);
            _ChangedNameFrostNova_Timer = 14000;
        }
        

        if (_ChangedNameFrostWard_Timer.Expired(diff))
        {
            DoCast(me, SPELL_FROSTWARD);
            _ChangedNameFrostWard_Timer = 68000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_seethrel(Creature *creature)
{
    return new boss_seethrelAI(creature);
}

#define SPELL_HAMSTRING             9080
#define SPELL_CLEAVE                15579
#define SPELL_MORTALSTRIKE          15708

struct boss_gloomrelAI : public ScriptedAI
{
    boss_gloomrelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameHamstring_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameMortalStrike_Timer;

    void Reset()
    {
        _ChangedNameHamstring_Timer.Reset(19000);
        _ChangedNameCleave_Timer.Reset(6000);
        _ChangedNameMortalStrike_Timer.Reset(10000);

        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameHamstring_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HAMSTRING);
            _ChangedNameHamstring_Timer = 14000;
        }
        
          

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 8000;
        }
        
          

        if (_ChangedNameMortalStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_MORTALSTRIKE);
            _ChangedNameMortalStrike_Timer = 12000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gloomrel(Creature *creature)
{
    return new boss_gloomrelAI(creature);
}

#define GOSSIP_ITEM_TEACH_1 16108
#define GOSSIP_ITEM_TEACH_2 16109
#define GOSSIP_ITEM_TRIBUTE 16110

bool GossipHello_boss_gloomrel(Player *player, Creature *creature)
{
    if (player->GetQuestRewardStatus(4083) == 1 && player->GetSkillValue(SKILL_MINING) >= 230 && !player->HasSpell(14891))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_TEACH_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (player->GetQuestRewardStatus(4083) == 0 && player->GetSkillValue(SKILL_MINING) >= 230)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_TRIBUTE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->SEND_GOSSIP_MENU(2601, creature->GetGUID());
    return true;
}

bool GossipSelect_boss_gloomrel(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    static uint64 SpectralChaliceGUID = 0;
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_TEACH_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            player->SEND_GOSSIP_MENU(2606, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 11:
            player->CLOSE_GOSSIP_MENU();
            creature->CastSpell(player, 14894, false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16111), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            player->SEND_GOSSIP_MENU(2604, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 22:
            player->CLOSE_GOSSIP_MENU();
            if (!creature->GetMap()->GetGameObject(SpectralChaliceGUID))
            {
                GameObject *spectralChalice = creature->SummonGameObject(164869, 1232, -239, -85, 4.05, 0, 0, 0, 0, 0);
                if (spectralChalice)
                    SpectralChaliceGUID = spectralChalice->GetGUID();
            }
            break;
    }
    return true;
}

#define SPELL_SHADOWBOLTVOLLEY               17228
#define SPELL_IMMOLATE                       15505
#define SPELL_CURSEOFWEAKNESS                17227
#define SPELL_DEMONARMOR                     11735
#define SPELL_SUMMON_VOIDS                   15092
#define GO_CHEST_SEVEN                       169243

struct boss_doomrelAI : public ScriptedAI
{
    boss_doomrelAI(Creature *c) : ScriptedAI(c), voids(me)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    SummonList voids;
    Timer _ChangedNameShadowVolley_Timer;
    Timer _ChangedNameImmolate_Timer;
    Timer _ChangedNameCurseOfWeakness_Timer;
    Timer _ChangedNameDemonArmor_Timer;
    bool Voidwalkers;

    uint64 DoomGUID;

    void Reset()
    {
        _ChangedNameShadowVolley_Timer.Reset(10000);
        _ChangedNameImmolate_Timer.Reset(18000);
        _ChangedNameCurseOfWeakness_Timer.Reset(5000);
        _ChangedNameDemonArmor_Timer.Reset(16000);
        Voidwalkers = false;

        DoomGUID = 0;

        me->setFaction(FACTION_NEUTRAL);
    }

    void EnterCombat(Unit *who)
    {
    }

    void JustSummoned(Creature* summoned)
    {
        voids.Summon(summoned);
        summoned->AI()->AttackStart(me->GetVictim());
    }

    void EnterEvadeMode()
    {
        voids.DespawnAll();

        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *slayer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_TOMB_OF_SEVEN, DONE);

        slayer->SummonGameObject(GO_CHEST_SEVEN, 1265.96f, -284.121f, -78.2191, 3.8531f, 0, 0, 0, 0, 0);
    }

    void DoAction(const int32 param)
    {
        switch (param)
        {
            case 1:
                DoomGUID = pInstance->GetData64(DATA_ANGERREL);
                break;
            case 2:
                DoomGUID = pInstance->GetData64(DATA_SEETHREL);
                break;
            case 3:
                DoomGUID = pInstance->GetData64(DATA_DOPEREL);
                break;
            case 4:
                DoomGUID = pInstance->GetData64(DATA_GLOOMREL);
                break;
            case 5:
                DoomGUID = pInstance->GetData64(DATA_VILEREL);
                break;
            case 6:
                DoomGUID = pInstance->GetData64(DATA_HATEREL);
                break;
            default:
                me->setFaction(FACTION_HOSTILE);
                DoZoneInCombat();
                return;
        }

        if (Creature *boss = me->GetCreature(DoomGUID))
        {
            boss->setFaction(FACTION_HOSTILE);
            boss->AI()->DoZoneInCombat();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameShadowVolley_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SHADOWBOLTVOLLEY);
            _ChangedNameShadowVolley_Timer = 12000;
        }
        

        if (_ChangedNameImmolate_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(target, SPELL_IMMOLATE);

            _ChangedNameImmolate_Timer = 25000;
        }
        

        if (_ChangedNameCurseOfWeakness_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CURSEOFWEAKNESS);
            _ChangedNameCurseOfWeakness_Timer = 45000;
        }
        

        if (_ChangedNameDemonArmor_Timer.Expired(diff))
        {
            DoCast(me, SPELL_DEMONARMOR);
            _ChangedNameDemonArmor_Timer = 300000;
        }
        

        //Summon Voidwalkers
        if (!Voidwalkers && me->GetHealthPercent() < 51 )
        {
            DoCast(me->GetVictim(), SPELL_SUMMON_VOIDS);
            Voidwalkers = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_doomrel(Creature *creature)
{
    return new boss_doomrelAI(creature);
}

#define GOSSIP_ITEM_CHALLENGE   16112
#define SAY_START               -1230038 // "You have challenged the Seven, and now you will die!"

bool GossipHello_boss_doomrel(Player *player, Creature *creature)
{
    ScriptedInstance *pInstance = (creature->GetInstanceData());

    if (pInstance->GetData(TYPE_TOMB_OF_SEVEN) == NOT_STARTED || pInstance->GetData(TYPE_TOMB_OF_SEVEN) == FAIL)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_CHALLENGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(2601, creature->GetGUID());
    }

    return true;
}

bool GossipSelect_boss_doomrel(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    ScriptedInstance *pInstance = (creature->GetInstanceData());

    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        DoScriptText(SAY_START, creature, player);
        pInstance->SetData(TYPE_TOMB_OF_SEVEN, IN_PROGRESS);
    }

    return true;
}

void AddSC_boss_tomb_of_seven()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_angerrel";
    newscript->GetAI = &GetAI_boss_angerrel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_doperel";
    newscript->GetAI = &GetAI_boss_doperel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_haterel";
    newscript->GetAI = &GetAI_boss_haterel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_vilerel";
    newscript->GetAI = &GetAI_boss_vilerel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_seethrel";
    newscript->GetAI = &GetAI_boss_seethrel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_gloomrel";
    newscript->GetAI = &GetAI_boss_gloomrel;
    newscript->pGossipHello = &GossipHello_boss_gloomrel;
    newscript->pGossipSelect = &GossipSelect_boss_gloomrel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_doomrel";
    newscript->GetAI = &GetAI_boss_doomrel;
    newscript->pGossipHello = &GossipHello_boss_doomrel;
    newscript->pGossipSelect = &GossipSelect_boss_doomrel;
    newscript->RegisterSelf();
}


