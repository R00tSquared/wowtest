// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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

#include "precompiled.h"
#include "def_blackrock_depths.h"

#define BOSS_THELDREN                   16059

#define GO_ARENA_SPOILS                 211085

//Spells
#define SPELL_DRINK_HEALING_POTION      15503
//theldren
#define SPELL_BATTLE_SHOUT              11551
#define SPELL_DEMORALIZING_SHOUT        11556
#define SPELL_DISARM                    27581
#define SPELL_FRIGHTENING_SHOUT         19134
#define SPELL_HAMSTRING                 27584
#define SPELL_INTERCEPT                 20617
#define SPELL_MORTAL_STRIKE             27580
//malgen_longspear
#define SPELL_FREEZING_TRAP             27753
#define SPELL_CONCLUSSIVE_SHOT          27634
#define SPELL_AIMED_SHOT                30614
#define SPELL_MULTI_SHOT                20735
#define SPELL_SHOOT                     6660
#define SPELL_WING_CLIP                 44286
//Lefty
#define SPELL_KNOCKDOWN                 7095
#define SPELL_SNAP_KICK                 27620
#define SPELL_FFFEHT                    27673
//Rotfang
#define SPELL_EVISCERATE                31016
#define SPELL_GOUGE                     24698
#define SPELL_KICK                      11978
#define SPELL_KIDNEY_SHOT               27615
#define SPELL_SINISTER_STRIKE           26862
#define SPELL_SLOWING_POISON            14897
#define SPELL_VANISH                    44290
//Va'jashni
#define SPELL_DISPEL_MAGIC              988
#define SPELL_FLASH_HEAL                27608
#define SPELL_PW_SHIELD                 20697
#define SPELL_RENEW                     23895
#define SPELL_SW_PAIN                   10894
//Volida
#define SPELL_BLINK                     14514
#define SPELL_BLIZZARD                  27618
#define SPELL_CONE_OF_COLD              12557
#define SPELL_FROST_NOVA                15063
#define SPELL_FROSTBOLT                 36990
#define SPELL_ICE_BLOCK                 45439
//Snokh Blackspine
#define SPELL_FLAMESTRIKE               11829
#define SPELL_SCORCH                    13878
#define SPELL_BLAST_WAVE                38064
#define SPELL_PYROBLAST                 17273
#define SPELL_POLYMORPH                 13323
//Korv
#define SPELL_FROST_SHOCK               12548
#define SPELL_WAR_STOMP                 46026
#define SPELL_WINDFURY_TOTEM            27621
#define SPELL_EARTHBIND_TOTEM           15786
#define SPELL_LESSER_HEALING_WAVE       10468
#define SPELL_PURGE                     8012
//Rezznik
#define SPELL_RECOMBOBULATE             27677
#define SPELL_DARK_IRON_BOMB            19784
#define SPELL_GOBLIN_DRAGON_GUN         44272
#define SPELL_EXPLOSIVE_SHEEP           8209
#define SPELL_SUMMON_ADRAGONLING        27602

static uint32 AddEntryList[9]=
{
    16049,      //Lefty
    16050,      //Rotfang
    16052,      //Malgen Longspear
    16055,      //Vajashni
    16058,      //Volida
    16051,      //Snokh Blackspine
    16053,      //Korv
    16054,      //Rezznik
    16095       ///Gnashjaw     - PET
};

float ArenaLocations[5][3]=
{
    {592.6309, -179.561, -53.90},
    {594.6309, -178.061, -53.90},
    {588.6309, -182.561, -53.90},
    {586.6309, -184.061, -53.90},
    {590.6309, -181.061, -53.90}        // theldren

};
float Orientation = 5.33;

//////////////////////
//Theldren
//////////////////////
struct boss_theldrenAI : public ScriptedAI
{
    boss_theldrenAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = me->GetInstanceData();
        SelectAddEntry();
        for (uint8 i = 0; i < 4; ++i)
            AddGUID[i] = 0;
    }

    ScriptedInstance* pInstance;
    uint64 AddGUID[4];
    uint32 AddEntry[4];

    Timer _ChangedNameBattleShout_Timer;
    Timer _ChangedNameDemoralizingShout_Timer;
    Timer _ChangedNameDisarm_Timer;
    Timer _ChangedNameFrighteningShout_Timer;
    Timer _ChangedNameHamstring_Timer;
    Timer _ChangedNameIntercept_Timer;
    Timer _ChangedNameMortalStrike_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        if (pInstance && pInstance->GetData(TYPE_THELDREN)!= DONE)
            pInstance->SetData(TYPE_THELDREN, NOT_STARTED);

        _ChangedNameBattleShout_Timer.Reset(6000);
        _ChangedNameDemoralizingShout_Timer.Reset(3000);
        _ChangedNameDisarm_Timer.Reset(1);
        _ChangedNameFrighteningShout_Timer.Reset(2000);
        _ChangedNameHamstring_Timer.Reset(5000);
        _ChangedNameIntercept_Timer.Reset(7000);
        _ChangedNameMortalStrike_Timer.Reset(6000);
        DrinkHealingPotion_Used = false;
        SpawnAdds();
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();

        for (uint8 i = 0; i < 4; ++i)
        {
            Unit* Temp = Unit::GetUnit((*me),AddGUID[i]);
            if (Temp && Temp->isAlive())
                ((Creature*)Temp)->AI()->AttackStart(me->GetVictim());
            else
            {
                EnterEvadeMode();
                break;
            }
        }
    }

    void JustDied(Unit* victim)
    {
        if (pInstance)
            pInstance->SetData(TYPE_THELDREN, DONE);
        victim->SummonGameObject(GO_ARENA_SPOILS, 596.664, -188.699, -54.1551, 5.67734, 0, 0, 0.298313, -0.954468, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameBattleShout_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_BATTLE_SHOUT);
            _ChangedNameBattleShout_Timer = 10000;
        }
           

        if (_ChangedNameDemoralizingShout_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_DEMORALIZING_SHOUT);
            _ChangedNameDemoralizingShout_Timer = 120000;
        }
        

        if (_ChangedNameDisarm_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_DISARM);
            _ChangedNameDisarm_Timer = 60000;
        }
        
            
        if (_ChangedNameFrighteningShout_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_FRIGHTENING_SHOUT);
            _ChangedNameFrighteningShout_Timer = 30000;
        }
           
        if (_ChangedNameHamstring_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_HAMSTRING);
            _ChangedNameHamstring_Timer = 30000;
        }
        
            
        if (_ChangedNameIntercept_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_INTERCEPT);
            _ChangedNameIntercept_Timer = 25000;
        }
        
            
        if (_ChangedNameMortalStrike_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
            _ChangedNameMortalStrike_Timer = 15000;
        }
        
            

        if (!DrinkHealingPotion_Used && HealthBelowPct(50))
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void SelectAddEntry()
    {
        std::vector<uint32> AddList;

        for (uint8 i = 0; i < 8; ++i)
            AddList.push_back(AddEntryList[i]);

        while (AddList.size() > 4)
            AddList.erase(AddList.begin()+rand()%AddList.size());

        uint8 i = 0;
        for (std::vector<uint32>::iterator itr = AddList.begin(); itr != AddList.end(); ++itr, ++i)
            AddEntry[i] = *itr;
    }

    void SpawnAdds()
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            Creature *pCreature = (Unit::GetCreature((*me), AddGUID[i]));
            if (!pCreature || !pCreature->isAlive())
            {
                if (pCreature) pCreature->setDeathState(DEAD);
                pCreature = me->SummonCreature(AddEntry[i], ArenaLocations[i][0], ArenaLocations[i][1], ArenaLocations[i][2], Orientation, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pCreature) AddGUID[i] = pCreature->GetGUID();
            }
            else
            {
                pCreature->AI()->EnterEvadeMode();
                pCreature->Relocate(ArenaLocations[i][0], ArenaLocations[i][1], ArenaLocations[i][2], Orientation);
                pCreature->StopMoving();
            }
        }
    }
};

CreatureAI* GetAI_boss_theldren(Creature *creature)
{
    return new boss_theldrenAI (creature);
}

//////////////////////
///add malgen_longspear
//////////////////////
struct boss_malgen_longspearAI : public ScriptedAI
{
    boss_malgen_longspearAI(Creature *c) : ScriptedAI(c)
    {
        PetGUID = 0;
    }

    ScriptedInstance* pInstance;
    uint64 PetGUID;

    Timer _ChangedNameFreezingTrap_Timer;
    Timer _ChangedNameAimedShot_Timer;
    Timer _ChangedNameConclussiveShot_Timer;
    Timer _ChangedNameMultiShot_Timer;
    Timer _ChangedNameShoot_Timer;
    Timer _ChangedNameWingClip_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameFreezingTrap_Timer.Reset(60000);
        _ChangedNameAimedShot_Timer.Reset(1);
        _ChangedNameConclussiveShot_Timer.Reset(1000);
        _ChangedNameMultiShot_Timer.Reset(2000);
        _ChangedNameShoot_Timer.Reset(1);
        _ChangedNameWingClip_Timer.Reset(9000);
        DrinkHealingPotion_Used = false;
        SpawnPet();
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();

        Unit* Temp = Unit::GetUnit((*me),PetGUID);
        if (Temp && Temp->isAlive())
            ((Creature*)Temp)->AI()->AttackStart(me->GetVictim());
        else
        {
            EnterEvadeMode();
        }
    }

    void JustDied(Unit* victim)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        if (_ChangedNameFreezingTrap_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_FREEZING_TRAP);
            _ChangedNameFreezingTrap_Timer = 60000;
        }
        
            
        if (_ChangedNameAimedShot_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_AIMED_SHOT);
            _ChangedNameAimedShot_Timer = 10000;
        }
        
           
        if (_ChangedNameConclussiveShot_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_CONCLUSSIVE_SHOT);
            _ChangedNameConclussiveShot_Timer = 8000;
        }
        
        if (_ChangedNameMultiShot_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_MULTI_SHOT);
            _ChangedNameMultiShot_Timer = 5000;
        }
        
        if (_ChangedNameShoot_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SHOOT);
            _ChangedNameShoot_Timer = 1500;
        }
        
        if (_ChangedNameWingClip_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_WING_CLIP, true);
            _ChangedNameWingClip_Timer = 20000;
        }
        

        if (!DrinkHealingPotion_Used && HealthBelowPct(50))
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void SpawnPet()
    {
        Creature *pPet = (Unit::GetCreature((*me), PetGUID));
        if (!pPet || !pPet->isAlive())
        {
            if (pPet) pPet->setDeathState(DEAD);
            pPet = me->SummonCreature(AddEntryList[8], me->GetPositionX(), me->GetPositionY()+2, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
            if (pPet) PetGUID = pPet->GetGUID();
        }
        else
        {
            pPet->AI()->EnterEvadeMode();
            pPet->Relocate(me->GetPositionX(), me->GetPositionY()+2, me->GetPositionZ(), me->GetOrientation());
            pPet->StopMoving();
        }
    }

};

CreatureAI* GetAI_boss_malgen_longspear(Creature *creature)
{
    return new boss_malgen_longspearAI (creature);
}

//////////////////////
///add lefty
//////////////////////
struct boss_leftyAI : public ScriptedAI
{
    boss_leftyAI(Creature *c) : ScriptedAI(c)
    {
    }

    ScriptedInstance* pInstance;
    uint64 PetGUID;

    Timer _ChangedNameKnockdown_Timer;
    Timer _ChangedNameSnapKick_Timer;
    Timer _ChangedNameFFFEHT_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameKnockdown_Timer.Reset(6000);
        _ChangedNameSnapKick_Timer.Reset(1);
        _ChangedNameFFFEHT_Timer.Reset(1000);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {
    }

    void JustDied(Unit* victim)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameKnockdown_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_KNOCKDOWN);
            _ChangedNameKnockdown_Timer = 30000;
        }
        

        if (_ChangedNameSnapKick_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SNAP_KICK);
            _ChangedNameSnapKick_Timer = 15000;
        }
        

        if (_ChangedNameFFFEHT_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_FFFEHT);
            _ChangedNameFFFEHT_Timer = 30000;
        }
          

        if(!DrinkHealingPotion_Used && HealthBelowPct(50))
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lefty(Creature *_Creature)
{
    return new boss_leftyAI (_Creature);
}

//////////////////////
///add rotfang
//////////////////////
struct boss_rotfangAI : public ScriptedAI
{
    boss_rotfangAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer _ChangedNameEviscerate_Timer;
    Timer _ChangedNameGouge_Timer;
    Timer _ChangedNameKick_Timer;
    Timer _ChangedNameKidneyShot_Timer;
    Timer _ChangedNameSinisterStrike_Timer;
    Timer _ChangedNameSlowingPoison_Timer;
    Timer _ChangedNameVanish_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameEviscerate_Timer.Reset(6000);
        _ChangedNameGouge_Timer.Reset(3000);
        _ChangedNameKick_Timer.Reset(1);
        _ChangedNameKidneyShot_Timer.Reset(2000);
        _ChangedNameSinisterStrike_Timer.Reset(5000);
        _ChangedNameSlowingPoison_Timer.Reset(7000);
        _ChangedNameVanish_Timer.Reset(6000);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameEviscerate_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_EVISCERATE);
            _ChangedNameEviscerate_Timer = 10000;
        }
        

        if (_ChangedNameGouge_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_GOUGE);
            _ChangedNameGouge_Timer = 120000;
        }
        
        if (_ChangedNameKick_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_KICK);
            _ChangedNameKick_Timer = 60000;
        }
        
        if (_ChangedNameKidneyShot_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_KIDNEY_SHOT);
            _ChangedNameKidneyShot_Timer = 30000;
        }
        
        if (_ChangedNameSinisterStrike_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SINISTER_STRIKE);
            _ChangedNameSinisterStrike_Timer = 30000;
        }
        
        if (_ChangedNameSlowingPoison_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SLOWING_POISON);
            _ChangedNameSlowingPoison_Timer = 25000;
        }
        
        if (_ChangedNameVanish_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_VANISH);
            _ChangedNameVanish_Timer = 15000;
        }
       
            

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_rotfang(Creature *_Creature)
{
    return new boss_rotfangAI (_Creature);
}

//////////////////////
///add Va'jashni
//////////////////////
struct boss_vajashniAI : public ScriptedAI
{
    boss_vajashniAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer _ChangedNameDispelMagic_Timer;
    Timer _ChangedNameFlashHeal_Timer;
    Timer _ChangedNamePWShield_Timer;
    Timer _ChangedNameRenew_Timer;
    Timer _ChangedNameSWPain_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameDispelMagic_Timer.Reset(2000);
        _ChangedNameFlashHeal_Timer.Reset(5000);
        _ChangedNamePWShield_Timer.Reset(1);
        _ChangedNameRenew_Timer.Reset(1000);
        _ChangedNameSWPain_Timer.Reset(1);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        if (_ChangedNameDispelMagic_Timer.Expired(diff))
        {
            if(rand()%2)
            {
                if(Unit* target = SelectLowestHpFriendly(50, 0))
                    AddSpellToCast(target, SPELL_DISPEL_MAGIC);
            }
            else if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_DISPEL_MAGIC), true))
            {
                AddSpellToCast(target, SPELL_DISPEL_MAGIC);
            }
            _ChangedNameDispelMagic_Timer = 6000;
        }
        
        if (_ChangedNameFlashHeal_Timer.Expired(diff))
        {
            if(Unit* target = SelectLowestHpFriendly(50, 1000))
            {
                AddSpellToCast(target, SPELL_FLASH_HEAL);
                _ChangedNameFlashHeal_Timer = 5000;
            }
        }
        
        if (_ChangedNamePWShield_Timer.Expired(diff))
        {
            if(Unit* target = SelectLowestHpFriendly(50, 0))
            {
                AddSpellToCast(target, SPELL_PW_SHIELD);
                _ChangedNamePWShield_Timer = 20000;
            }
        }
        
        if (_ChangedNameRenew_Timer.Expired(diff))
        {
            if(Unit* target = SelectLowestHpFriendly(50, 0))
            {
                AddSpellToCast(target, SPELL_RENEW);
                _ChangedNameRenew_Timer = 10000;
            }
        }
        
           
        if (_ChangedNameSWPain_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SW_PAIN), true))
            {
                AddSpellToCast(target, SPELL_SW_PAIN);
                _ChangedNameSWPain_Timer = 7000;
            }
        }
        
          

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_vajashni(Creature *_Creature)
{
    return new boss_vajashniAI (_Creature);
}

//////////////////////
///add Volida
//////////////////////
struct boss_volidaAI : public ScriptedAI
{
    boss_volidaAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer _ChangedNameBlink_Timer;
    Timer _ChangedNameBlizzard_Timer;
    Timer _ChangedNameConeOfCold_Timer;
    Timer _ChangedNameFrostNova_Timer;
    Timer _ChangedNameFrostBolt_Timer;
    Timer _ChangedNameIceBlock_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameBlink_Timer.Reset(2000);
        _ChangedNameBlizzard_Timer.Reset(5000);
        _ChangedNameConeOfCold_Timer.Reset(1);
        _ChangedNameFrostNova_Timer.Reset(1000);
        _ChangedNameFrostBolt_Timer.Reset(1);
        _ChangedNameIceBlock_Timer.Reset(1);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        if (_ChangedNameBlink_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_BLINK);
            _ChangedNameBlink_Timer = 30000;
        }
        
        if (_ChangedNameBlizzard_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_BLIZZARD), true))
            {
                AddSpellToCast(target, SPELL_BLIZZARD);
                _ChangedNameBlizzard_Timer = 20000;
            }
        }
        
            
        if (_ChangedNameConeOfCold_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_CONE_OF_COLD);
            _ChangedNameConeOfCold_Timer = 15000;
        }
        
        if (_ChangedNameFrostNova_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_FROST_NOVA);
            _ChangedNameFrostNova_Timer = 25000;
        }
        
        if (_ChangedNameFrostBolt_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_FROSTBOLT);
            _ChangedNameFrostBolt_Timer = 3500;
        }
        
        if (HealthBelowPct(20) && _ChangedNameIceBlock_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_ICE_BLOCK);
            _ChangedNameIceBlock_Timer = 30000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_volida(Creature *_Creature)
{
    return new boss_volidaAI (_Creature);
}

//////////////////////
///add Snokh Blackspine
//////////////////////
struct boss_snokhAI : public ScriptedAI
{
    boss_snokhAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer _ChangedNameBlink_Timer2;
    Timer _ChangedNameFlameStrike_Timer;
    Timer _ChangedNameScorch_Timer;
    Timer _ChangedNameBlastWave_Timer;
    Timer _ChangedNamePyroblast_Timer;
    Timer _ChangedNamePolymorph_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameBlink_Timer2.Reset(2000);
        _ChangedNameFlameStrike_Timer.Reset(1000);
        _ChangedNameScorch_Timer.Reset(1);
        _ChangedNameBlastWave_Timer.Reset(1000);
        _ChangedNamePyroblast_Timer.Reset(5000);
        _ChangedNamePolymorph_Timer.Reset(2000);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        if (_ChangedNameBlink_Timer2.Expired(diff))
        {
            AddSpellToCast(me, SPELL_BLINK);
            _ChangedNameBlink_Timer2 = 30000;
        }
        
        if (_ChangedNameFlameStrike_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_FLAMESTRIKE), true))
            {
                AddSpellToCast(target, SPELL_BLIZZARD);
                _ChangedNameFlameStrike_Timer = 7000;
            }
        }
        

        if (_ChangedNameScorch_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SCORCH);
            _ChangedNameScorch_Timer = 5000;
        }
        
            
        if (_ChangedNameBlastWave_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_BLAST_WAVE);
            _ChangedNameBlastWave_Timer = 15000;
        }
        
        if (_ChangedNamePyroblast_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_PYROBLAST);
            _ChangedNamePyroblast_Timer = 25000;
        }
        
          
        if (_ChangedNamePolymorph_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_POLYMORPH);
            _ChangedNamePolymorph_Timer = 15000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_snokh(Creature *_Creature)
{
    return new boss_snokhAI (_Creature);
}

//////////////////////
///add Korv
//////////////////////
struct boss_korvAI : public ScriptedAI
{
    boss_korvAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer _ChangedNameFrostShock_Timer;
    Timer _ChangedNameWarStamp_Timer;
    Timer _ChangedNameWindfuryT_Timer;
    Timer _ChangedNameEarthbindT_Timer;
    Timer _ChangedNameLesserHealing_Timer;
    Timer _ChangedNamePurge_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameFrostShock_Timer.Reset(2000);
        _ChangedNameWarStamp_Timer.Reset(1000);
        _ChangedNameWindfuryT_Timer.Reset(1);
        _ChangedNameEarthbindT_Timer.Reset(1);
        _ChangedNameLesserHealing_Timer.Reset(5000);
        _ChangedNamePurge_Timer.Reset(2000);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameFrostShock_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_FROST_SHOCK);
            _ChangedNameFrostShock_Timer = 10000;
        }
        
        if (_ChangedNameWarStamp_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_WAR_STOMP);
            _ChangedNameWarStamp_Timer = 15000;
        }
        
        if (_ChangedNamePurge_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_PURGE);
            _ChangedNamePurge_Timer = 7000;
        }
        
        if (_ChangedNameLesserHealing_Timer.Expired(diff))
        {
            if(Unit* target = SelectLowestHpFriendly(50, 1000))
            {
                AddSpellToCast(target,SPELL_LESSER_HEALING_WAVE);
                _ChangedNameLesserHealing_Timer = 6500;
            }
        }
        
            
        if (_ChangedNameWindfuryT_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_WINDFURY_TOTEM);
            _ChangedNameWindfuryT_Timer = 25000;
        }
        

        if (_ChangedNameEarthbindT_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_EARTHBIND_TOTEM);
            _ChangedNameEarthbindT_Timer = 25000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_korv(Creature *_Creature)
{
    return new boss_korvAI (_Creature);
}

//////////////////////
///add Rezznik
//////////////////////
struct boss_rezznikAI : public ScriptedAI
{
    boss_rezznikAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer _ChangedNameRecombobulate_Timer;
    Timer _ChangedNameDarkIronBomb_Timer;
    Timer _ChangedNameGoblinGragonGun_Timer;
    Timer _ChangedNameExplosiveSheep_Timer;
    Timer _ChangedNameSummonADragonling_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        _ChangedNameRecombobulate_Timer.Reset(1000);
        _ChangedNameDarkIronBomb_Timer.Reset(1000);
        _ChangedNameGoblinGragonGun_Timer.Reset(1);
        _ChangedNameExplosiveSheep_Timer.Reset(1);
        _ChangedNameSummonADragonling_Timer.Reset(5000);
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameRecombobulate_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_RECOMBOBULATE);
            _ChangedNameRecombobulate_Timer = 11000;
        }
        
        if (_ChangedNameDarkIronBomb_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_DARK_IRON_BOMB);
            _ChangedNameDarkIronBomb_Timer = 4000;
        }
        
        if (_ChangedNameGoblinGragonGun_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_GOBLIN_DRAGON_GUN);
            _ChangedNameGoblinGragonGun_Timer = 12000;
        }
        
        if (_ChangedNameExplosiveSheep_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_EXPLOSIVE_SHEEP);
            _ChangedNameExplosiveSheep_Timer = 20000;
        }
        
        if (_ChangedNameSummonADragonling_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SUMMON_ADRAGONLING);
            _ChangedNameSummonADragonling_Timer = 3600000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_rezznik(Creature *_Creature)
{
    return new boss_rezznikAI (_Creature);
}

void AddSC_boss_theldren()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_theldren";
    newscript->GetAI = &GetAI_boss_theldren;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_malgen_longspear";
    newscript->GetAI = &GetAI_boss_malgen_longspear;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lefty";
    newscript->GetAI = &GetAI_boss_lefty;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_rotfang";
    newscript->GetAI = &GetAI_boss_rotfang;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_vajashni";
    newscript->GetAI = &GetAI_boss_vajashni;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_volida";
    newscript->GetAI = &GetAI_boss_volida;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_snokh";
    newscript->GetAI = &GetAI_boss_snokh;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_korv";
    newscript->GetAI = &GetAI_boss_korv;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_rezznik";
    newscript->GetAI = &GetAI_boss_rezznik;
    newscript->RegisterSelf();
}

