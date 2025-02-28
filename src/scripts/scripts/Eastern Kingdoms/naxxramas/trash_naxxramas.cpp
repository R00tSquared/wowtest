// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* ScriptData
SDName: trash_naxxramas
SD%Complete: 100%(42/42)
SDComment: Trash NPC
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

#define AGGRO_RANGE     25.0

/* Content Data
    *** Abdomination Wing(100%)
        ** Patchwerk
            * Patchwork Golem
            * Bile Retcher
            * Sewage Slime
            * Sludge Belcher
            * Embalming Slime
            * Living Monstrosity
            * Surgical Assistant
            * Mad Scientist
        ** Grobbulus
            * Stitched Spewer
    *** Spider Wing(100%)
        ** Anub'Rekhan
            * Infectious Skitterer
            * Carrion Spinner
            * Dread Creeper
            * Venom Stalker
            * Crypt Reaver
            * Necro Stalker
        ** Grand Widow Faerlina
            * Naxxramas Cultist
            * Naxxramas Acolyte
            * Crypt Guard
    *** Deathknight Wing(100%)
        ** Instructor Razuvious
            * Risen Deathknight
            * Deathknight
            * Deathknight Captain
            * Necro Knight
            * Shade of Naxxramas
            * Dark Touched Warrior
            * Death Touched Warrior
            * Skeletal Smith
            * Bony Construct
            * Deathknight Cavalier
            * Skeletal Steed
            * Doom Touched Warrior
    *** Plague Wing(100%)
        ** Noth the Plaguebringer
            * Infectious Ghoul
            * Plague Slime
            * Stoneskin Gargoyle
        ** Heigan the Unclean
            * Mutated Grub
            * Plagued Bat
            * Plague Beast
        ** Loatheb
            * Eye Stalk
            * Rotting Maggot
            * Plagued Ghoul
    *** Other
        ** Tunnel(100%)
            * Necropolis Acolyte
            * Necro Knight Guardian
            * Deathknight Vindicator
*/

/*#################
# Abdomination Wing
#################*/

/****************
* Patchwork Golem - id 16017
*****************/

enum PatchworkGolem
{
    SPELL_PG_DISEASE_CLOUD  = 27793,
    SPELL_PG_CLEAVE         = 27794,
    SPELL_PG_WAR_STOMP      = 27758
};

struct mob_patchwork_golemAI : public ScriptedAI
{
    mob_patchwork_golemAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 CleaveTimer;
    uint32 WarStompTimer;

    void Reset()
    {
        DoCast(me, SPELL_PG_DISEASE_CLOUD, true);
        ClearCastQueue();
        CleaveTimer = 5000;
        WarStompTimer = 14000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_PG_CLEAVE, CAST_TANK);
            CleaveTimer = 5000;
        }
        else
            CleaveTimer -= diff;

        if(WarStompTimer < diff)
        {
            AddSpellToCast(SPELL_PG_WAR_STOMP, CAST_TANK);
            WarStompTimer = 14000;
        }
        else
            WarStompTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_patchwork_golem(Creature *_Creature)
{
    return new mob_patchwork_golemAI(_Creature);
}

/****************
* Bile Retcher - id 16018
*****************/

enum BileRetcher
{
    SPELL_BR_BILE_VOMIT     = 27807,
    SPELL_BR_SLAM           = 27862
};

struct mob_bile_retcherAI : public ScriptedAI
{
    mob_bile_retcherAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 BileVomitTimer;
    uint32 SlamTimer;

    void Reset()
    {
        ClearCastQueue();
        BileVomitTimer = urand(10000, 15000);
        SlamTimer = 5000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(BileVomitTimer < diff)
        {
            AddSpellToCast(SPELL_BR_BILE_VOMIT, CAST_TANK);
            BileVomitTimer = urand(10000, 15000);
        }
        else
            BileVomitTimer -= diff;

        if(SlamTimer < diff)
        {
            AddSpellToCast(SPELL_BR_SLAM, CAST_TANK);
            SlamTimer = 5000;
        }
        else
            SlamTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bile_retcher(Creature *_Creature)
{
    return new mob_bile_retcherAI(_Creature);
}

/****************
* Sewage Slime - id 16375
*****************/

enum SewageSlime
{
    SPELL_SS_DISEASE_CLOUD  = 28156
};

struct mob_sewage_slimeAI : public ScriptedAI
{
    mob_sewage_slimeAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(75.0); }

    void Reset()
    {
        DoCast(me, SPELL_SS_DISEASE_CLOUD, true);
        ClearCastQueue();
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_sewage_slime(Creature *_Creature)
{
    return new mob_sewage_slimeAI(_Creature);
}

/****************
* Sludge Belcher - id 16029
*****************/

enum SludgeBelcher
{
    SPELL_SB_DISEASE_BUFFET     = 27891,
    SPELL_SB_SPAWN_BILE_SLUDGE  = 27889,
    NPC_BILE_SLUDGE             = 16142
};

struct mob_sludge_belcherAI : public ScriptedAI
{
    mob_sludge_belcherAI(Creature *c) : ScriptedAI(c), summons(c) { me->SetAggroRange(AGGRO_RANGE); }

    SummonList summons;
    uint32 DiseaseBuffetTimer;
    uint32 SpawnBileSludge;

    void Reset()
    {
        ClearCastQueue();
        DiseaseBuffetTimer = 6000;
        SpawnBileSludge = 3000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }
    
    void JustDied(Unit* killer)
    {
        summons.DespawnAll();
    }

    void JustSummoned(Creature *summoned)
    {
        if(summoned)
            summons.Summon(summoned);

        if(summoned->GetEntry() == NPC_BILE_SLUDGE)
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target && summoned)
            {
                summoned->AI()->AttackStart(target);
                summoned->setFaction(me->getFaction());
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(DiseaseBuffetTimer < diff)
        {
            AddSpellToCast(SPELL_SB_DISEASE_BUFFET, CAST_TANK);
            DiseaseBuffetTimer = 6000;
        }
        else DiseaseBuffetTimer -= diff;

        if(SpawnBileSludge < diff)
        {
            AddSpellToCast(SPELL_SB_SPAWN_BILE_SLUDGE, CAST_TANK);
            SpawnBileSludge = 15000;
        }
        else SpawnBileSludge -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_sludge_belcher(Creature *_Creature)
{
    return new mob_sludge_belcherAI(_Creature);
}

/****************
* Embalming Slime - id 16024
*****************/

enum EmbalmingSlime
{
    SPELL_ES_EMBALMING_CLOUD    = 28322
};

struct mob_embalming_slimeAI : public ScriptedAI
{
    mob_embalming_slimeAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(50.0); }

    uint32 EmbalmingCloudTimer;

    void Reset()
    {
        ClearCastQueue();
        EmbalmingCloudTimer = 5000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(EmbalmingCloudTimer < diff)
        {
            AddSpellToCast(SPELL_ES_EMBALMING_CLOUD, CAST_TANK);
            EmbalmingCloudTimer = 5000;
        }
        else EmbalmingCloudTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_embalming_slime(Creature *_Creature)
{
    return new mob_embalming_slimeAI(_Creature);
}

/****************
* Living Monstrosity - id 16021
*****************/

enum LivingMonstrosity
{
    SPELL_LM_LIGHTING_TOTEM     = 28294,
    SPELL_LM_CHAIN_LIGHTING     = 27889,
    SPELL_LM_FEAR               = 27990,
    NPC_LM_TOTEM                = 16385
};

struct mob_living_monstrosityAI : public ScriptedAI
{
    mob_living_monstrosityAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 LightingTotemTimer;
    uint32 ChainLightingTimer;
    uint32 FearTimer;

    void Reset()
    {
        ClearCastQueue();
        LightingTotemTimer = 1000;
        ChainLightingTimer = urand(5000,10000);
        FearTimer = urand(6000,12000);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void JustSummoned(Creature *summoned)
    {
        if(summoned->GetEntry() == NPC_LM_TOTEM)
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target && summoned)
            {
                summoned->AI()->AttackStart(target);
                summoned->setFaction(me->getFaction());
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(LightingTotemTimer < diff)
        {
            AddSpellToCast(SPELL_LM_LIGHTING_TOTEM, CAST_NULL);
            LightingTotemTimer = 60000;
        }
        else LightingTotemTimer -= diff;

        if(ChainLightingTimer < diff)
        {
            AddSpellToCast(SPELL_LM_CHAIN_LIGHTING, CAST_TANK);
            ChainLightingTimer = urand(5000,10000);
        }
        else ChainLightingTimer -= diff;

        if(FearTimer < diff)
        {
            AddSpellToCast(SPELL_LM_FEAR, CAST_TANK);
            FearTimer = urand(6000,12000);
        }
        else FearTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_living_monstrosity(Creature *_Creature)
{
    return new mob_living_monstrosityAI(_Creature);
}

/****************
* Surgical Assistant - id 16022
*****************/

enum SurgicalAssistant
{
    SPELL_SA_MIND_FLAY  = 28310
};

struct mob_surgical_assistantAI : public ScriptedAI
{
    mob_surgical_assistantAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 MindFlayTimer;

    void Reset()
    {
        ClearCastQueue();
        MindFlayTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(MindFlayTimer < diff)
        {
            AddSpellToCast(SPELL_SA_MIND_FLAY, CAST_RANDOM);
            MindFlayTimer = 10000;
        }
        else MindFlayTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_surgical_assistant(Creature *_Creature)
{
    return new mob_surgical_assistantAI(_Creature);
}


/****************
* Mad Scientist - id 16020
*****************/

enum MadScientist
{
    SPELL_MS_GREAT_HEAL     = 28306,
    SPELL_MS_MANA_BURN      = 28301,
    NPC_LIVING_MONSTROSITY  = 16021
};

struct mob_mad_scientistAI : public ScriptedAI
{
    mob_mad_scientistAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 ManaBurnTimer;
    uint32 GreatHealTimer;

    void Reset()
    {
        ClearCastQueue();
        ManaBurnTimer = urand(7000,10000);
        GreatHealTimer = 10000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ManaBurnTimer < diff)
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true, POWER_MANA))
                AddSpellToCast(target, SPELL_MS_MANA_BURN, false, true);
            ManaBurnTimer = urand(7000,10000);
        }
        else ManaBurnTimer -= diff;
        
        if(GreatHealTimer < diff)
        {
            if(Unit* LivingMonstrosity = FindCreature(NPC_LIVING_MONSTROSITY, 15.0, me))
                AddSpellToCast(LivingMonstrosity, SPELL_MS_GREAT_HEAL, false, true);
            else
                AddSpellToCast(SPELL_MS_GREAT_HEAL, CAST_SELF);
            GreatHealTimer = 10000;
        } else GreatHealTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mad_scientist(Creature *_Creature)
{
    return new mob_mad_scientistAI(_Creature);
}

/****************
* Stitched Spewer - id 16025
*****************/

enum StitchedSpewer
{
    SPELL_SS_SLIME_BOLT     = 28311,
    SPELL_SS_SLIME_SHOOT    = 28318,
    SPELL_SS_KNOCKBACK      = 28405
};

struct mob_stitched_spewerAI : public ScriptedAI
{
    mob_stitched_spewerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 SlimeBoltTimer;
    uint32 SlimeShootTimer;
    uint32 KnockbackTimer;

    void Reset()
    {
        ClearCastQueue();
        SlimeBoltTimer  = 5000;
        SlimeShootTimer = urand(5000,6000);
        KnockbackTimer  = urand(3000,7000);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(SlimeBoltTimer < diff)
        {
            AddSpellToCast(SPELL_SS_SLIME_BOLT, CAST_TANK);
            SlimeBoltTimer = 5000;
        }
        else SlimeBoltTimer -= diff;
        
        if(SlimeShootTimer < diff)
        {
            AddSpellToCast(SPELL_SS_SLIME_SHOOT, CAST_TANK);
            SlimeShootTimer = urand(5000,6000);
        }
        else SlimeShootTimer -= diff;

        if(KnockbackTimer < diff)
        {
            AddSpellToCast(SPELL_SS_KNOCKBACK, CAST_TANK);
            KnockbackTimer = urand(3000,7000);
        }
        else KnockbackTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_stitched_spewer(Creature *_Creature)
{
    return new mob_stitched_spewerAI(_Creature);
}

/*###########
# Spider Wing
###########*/

/****************
* Infectious Skitterer - id 15977
*****************/

struct mob_infectious_skittererAI : public ScriptedAI
{
    mob_infectious_skittererAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    void Reset()
    {
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_infectious_skitterer(Creature *_Creature)
{
    return new mob_infectious_skittererAI(_Creature);
}

/****************
* Carrion Spinner - id 15975
*****************/

enum CarrionSpinner
{
    SPELL_CS_SPIDER_WEB     = 28434,
    SPELL_CS_POISON_SPRAY   = 30043
};

struct mob_carrion_spinnerAI : public ScriptedAI
{
    mob_carrion_spinnerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 SpiderWebTimer;
    uint32 PoisonSprayTimer;

    void Reset()
    {
        ClearCastQueue();
        SpiderWebTimer = 3000;
        PoisonSprayTimer = 7000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(SpiderWebTimer < diff)
        {
            AddSpellToCast(SPELL_CS_SPIDER_WEB, CAST_RANDOM);
            SpiderWebTimer = urand(3000, 10000);
        }
        else
            SpiderWebTimer -= diff;

        if(PoisonSprayTimer < diff)
        {
            AddSpellToCast(SPELL_CS_POISON_SPRAY, CAST_TANK);
            PoisonSprayTimer = 7000;
        }
        else
            PoisonSprayTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_carrion_spinner(Creature *_Creature)
{
    return new mob_carrion_spinnerAI(_Creature);
}

/****************
* Dread Creeper - id 15974
*****************/

enum DreadCreeper
{
    SPELL_DC_VEIL_OF_SHADOW = 28440
};

struct mob_dread_creeperAI : public ScriptedAI
{
    mob_dread_creeperAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 VeilOfShadowTimer;

    void Reset()
    {
        ClearCastQueue();
        VeilOfShadowTimer = 2000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(VeilOfShadowTimer < diff)
        {
            AddSpellToCast(SPELL_DC_VEIL_OF_SHADOW, CAST_RANDOM);
            VeilOfShadowTimer = urand(2000, 6000);
        }
        else
            VeilOfShadowTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_dread_creeper(Creature *_Creature)
{
    return new mob_dread_creeperAI(_Creature);
}

/****************
* Venom Stalker - id 15976
*****************/

enum VenomStalker
{
    SPELL_VS_POISON_CHARGE = 28431
};

struct mob_venom_stalkerAI : public ScriptedAI
{
    mob_venom_stalkerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 PoisonChargeTimer;

    void Reset()
    {
        ClearCastQueue();
        PoisonChargeTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(PoisonChargeTimer < diff)
        {
            AddSpellToCast(SPELL_VS_POISON_CHARGE, CAST_RANDOM);
            PoisonChargeTimer = urand(2000, 4000);
        }
        else
            PoisonChargeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_venom_stalker(Creature *_Creature)
{
    return new mob_venom_stalkerAI(_Creature);
}

/****************
* Crypt Reaver - id 15978
*****************/

enum CryptReaver
{
    SPELL_CR_CLEAVE             = 40504,
    SPELL_CR_VIRULENT_POISON    = 22412
};

struct mob_crypt_reaverAI : public ScriptedAI
{
    mob_crypt_reaverAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 CleaveTimer;
    uint32 VirulentPoisonTimer;

    void Reset()
    {
        ClearCastQueue();
        CleaveTimer = 3000;
        VirulentPoisonTimer = 2000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_CR_CLEAVE, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        }
        else
            CleaveTimer -= diff;

        if(VirulentPoisonTimer < diff)
        {
            AddSpellToCast(SPELL_CR_VIRULENT_POISON, CAST_RANDOM);
            VirulentPoisonTimer = 30000;
        }
        else
            VirulentPoisonTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_crypt_reaver(Creature *_Creature)
{
    return new mob_crypt_reaverAI(_Creature);
}

/****************
* Necro Stalker - id 16453
*****************/

enum NecroStalker
{
    SPELL_NS_POISON_CHARGE  = 28431
};

struct mob_necro_stalkerAI : public ScriptedAI
{
    mob_necro_stalkerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 PoisonChargeTimer;

    void Reset()
    {
        ClearCastQueue();
        PoisonChargeTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(PoisonChargeTimer < diff)
        {
            AddSpellToCast(SPELL_NS_POISON_CHARGE, CAST_RANDOM);
            PoisonChargeTimer = urand(2000, 4000);
        }
        else
            PoisonChargeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_necro_stalker(Creature *_Creature)
{
    return new mob_necro_stalkerAI(_Creature);
}

/****************
* Naxxramas Cultist - id 15980
*****************/

enum NaxxramasCultist
{
    SPELL_NC_SHADOW_BURST   = 28447,
    SPELL_NC_KNOCKBACK      = 19813
};

struct mob_naxxramas_cultistAI : public ScriptedAI
{
    mob_naxxramas_cultistAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 ShadowBurstTimer;
    uint32 KnockbackTimer;

    void Reset()
    {
        ClearCastQueue();
        ShadowBurstTimer = 3000;
        KnockbackTimer = 5000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ShadowBurstTimer < diff)
        {
            AddSpellToCast(SPELL_NC_SHADOW_BURST, CAST_NULL);
            ShadowBurstTimer = urand(3000, 5000);
        }
        else
            ShadowBurstTimer -= diff;

        if(KnockbackTimer < diff)
        {
            AddSpellToCast(SPELL_NC_KNOCKBACK, CAST_RANDOM);
            KnockbackTimer = 5000;
        }
        else
            KnockbackTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_naxxramas_cultist(Creature *_Creature)
{
    return new mob_naxxramas_cultistAI(_Creature);
}

/****************
* Naxxramas Acolyte - id 15981
*****************/

enum NaxxramasAcolyte
{
    SPELL_NA_ARCANE_EXPLOSION   = 15253,
    SPELL_NA_SHADOWBOLT_VOLLEY  = 28448
};

struct mob_naxxramas_acolyteAI : public ScriptedAI
{
    mob_naxxramas_acolyteAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 ArcaneExplosionTimer;
    uint32 ShadowboltVolleyTimer;

    void Reset()
    {
        ClearCastQueue();
        ArcaneExplosionTimer = 2000;
        ShadowboltVolleyTimer = 4000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ArcaneExplosionTimer < diff)
        {
            AddSpellToCast(SPELL_NA_ARCANE_EXPLOSION, CAST_NULL);
            ArcaneExplosionTimer = urand(2000, 5000);
        }
        else
            ArcaneExplosionTimer -= diff;

        if(ShadowboltVolleyTimer < diff)
        {
            AddSpellToCast(SPELL_NA_SHADOWBOLT_VOLLEY, CAST_TANK);
            ShadowboltVolleyTimer = urand(4000, 6000);
        }
        else
            ShadowboltVolleyTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_naxxramas_acolyte(Creature *_Creature)
{
    return new mob_naxxramas_acolyteAI(_Creature);
}

/****************
* Crypt Guard - id 16573
*****************/

enum CryptGuard
{
    SPELL_CG_ACID_SPIT  = 28969,
    SPELL_CG_CLEAVE     = 40504,
    SPELL_FRENZY        = 8269
};

struct mob_crypt_guardAI : public ScriptedAI
{
    mob_crypt_guardAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 AcidSplitTimer;
    uint32 CleaveTimer;
    bool FrenzyCasted;

    void Reset()
    {
        ClearCastQueue();
        AcidSplitTimer = 2000;
        CleaveTimer = 3000;
        FrenzyCasted = false;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(AcidSplitTimer < diff)
        {
            AddSpellToCast(SPELL_CG_ACID_SPIT, CAST_RANDOM);
            AcidSplitTimer = 10000;
        }
        else
            AcidSplitTimer -= diff;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_CG_CLEAVE, CAST_TANK);
            CleaveTimer = urand(3000, 6000);
        }
        else
            CleaveTimer -= diff;
        
        if(HealthBelowPct(30) && !FrenzyCasted)
        {
            AddSpellToCast(SPELL_FRENZY, CAST_SELF);
            FrenzyCasted = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_crypt_guard(Creature *_Creature)
{
    return new mob_crypt_guardAI(_Creature);
}

/*###########
# Plague Wing
###########*/

/****************
* Infectious Ghoul - id 16244
*****************/

enum InfectiousGhoul
{
    SPELL_IG_FLESH_ROT  = 29915,
    SPELL_IG_REND       = 13738,
    SPELL_IG_ENRAGE     = 24318
};

struct mob_infectious_ghoulAI : public ScriptedAI
{
    mob_infectious_ghoulAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 FleshRotTimer;
    uint32 RendTimer;
    bool Enraged;

    void Reset()
    {
        FleshRotTimer   = 2000;
        RendTimer       = 3000;
        Enraged         = false;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(FleshRotTimer < diff)
        {
            AddSpellToCast(SPELL_IG_FLESH_ROT, CAST_RANDOM);
            FleshRotTimer = urand(2000, 12000);
        }
        else
            FleshRotTimer -= diff;

        if(RendTimer < diff)
        {
            AddSpellToCast(SPELL_IG_REND, CAST_RANDOM);
            RendTimer = urand(3000, 15000);
        }
        else
            RendTimer -= diff;
        
        if(HealthBelowPct(30) && !Enraged)
        {
            AddSpellToCast(SPELL_IG_ENRAGE, CAST_SELF);
            Enraged = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_infectious_ghoul(Creature *_Creature)
{
    return new mob_infectious_ghoulAI(_Creature);
}

/****************
* Plague Slime - id 16243
*****************/

enum PlagueSlime
{
    SPELL_PS_PLAGUE_SLIME_BLACK = 28987,
    SPELL_PS_PLAGUE_SLIME_BLUE  = 28988,
    SPELL_PS_PLAGUE_SLIME_GREEN = 28989,
    SPELL_PS_PLAGUE_SLIME_RED   = 28990
};

struct mob_plague_slimeAI : public ScriptedAI
{
    mob_plague_slimeAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 TransformTimer;

    void Reset()
    {
        TransformTimer = 3000;
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, false);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, false);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, false);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, false);
        me->RemoveAllAurasNotCreatureAddon();
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(TransformTimer < diff)
        {
            uint32 SpellID = RAND(SPELL_PS_PLAGUE_SLIME_BLACK, SPELL_PS_PLAGUE_SLIME_BLUE, SPELL_PS_PLAGUE_SLIME_GREEN, SPELL_PS_PLAGUE_SLIME_RED);
            AddSpellToCast(SpellID, CAST_SELF);
            switch(SpellID)
            {
                case SPELL_PS_PLAGUE_SLIME_BLACK: me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true); break;
                case SPELL_PS_PLAGUE_SLIME_BLUE:  me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);  break;
                case SPELL_PS_PLAGUE_SLIME_GREEN: me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true); break;
                case SPELL_PS_PLAGUE_SLIME_RED:   me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);   break;
            }
            TransformTimer = urand(2000, 12000);
        }
        else
            TransformTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_plague_slime(Creature *_Creature)
{
    return new mob_plague_slimeAI(_Creature);
}

/****************
* Stoneskin Gargoyle - id 16168
*****************/

enum StoneskinGargolye
{
    SPELL_SG_ACID_VOLLEY    = 29325,
    SPELL_SG_STONESKIN      = 28995
};

struct mob_stoneskin_gargoyleAI : public ScriptedAI
{
    mob_stoneskin_gargoyleAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 AcidVolleyTimer;
    uint32 StoneskinTimer;

    void Reset()
    {
        AcidVolleyTimer = 1000;
        StoneskinTimer  = 45000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(AcidVolleyTimer < diff)
        {
            AddSpellToCast(SPELL_SG_ACID_VOLLEY, CAST_TANK);
            AcidVolleyTimer = 10000;
        }
        else
            AcidVolleyTimer -= diff;

        if(StoneskinTimer < diff)
        {
            AddSpellToCast(SPELL_SG_STONESKIN, CAST_SELF);
            StoneskinTimer = 60000;
        }
        else
            StoneskinTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_stoneskin_gargoyle(Creature *_Creature)
{
    return new mob_stoneskin_gargoyleAI(_Creature);
}

/****************
* Mutated Grub - id 16297
*****************/

enum MutatedGrub
{
    SPELL_MG_SLIME_BURST = 30109
};

struct mob_mutated_grubAI : public ScriptedAI
{
    mob_mutated_grubAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 SlimeBurstTimer;

    void Reset()
    {
        SlimeBurstTimer = urand(1000, 4000);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(SlimeBurstTimer < diff)
        {
            AddSpellToCast(SPELL_MG_SLIME_BURST, CAST_NULL);
            SlimeBurstTimer = urand(1000, 4000);
        }
        else
            SlimeBurstTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mutated_grub(Creature *_Creature)
{
    return new mob_mutated_grubAI(_Creature);
}

/****************
* Plagued Bat - id 16037
*****************/

enum PlaguedBat
{
    SPELL_PB_PUTRID_BITE = 30113
};

struct mob_plagued_batAI : public ScriptedAI
{
    mob_plagued_batAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 PutridBiteTimer;

    void Reset()
    {
        PutridBiteTimer = 9000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(PutridBiteTimer < diff)
        {
            AddSpellToCast(SPELL_PB_PUTRID_BITE, CAST_TANK);
            PutridBiteTimer = 40000;
        }
        else
            PutridBiteTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_plagued_bat(Creature *_Creature)
{
    return new mob_plagued_batAI(_Creature);
}

/****************
* Plague Beast - id 16034
*****************/

enum PlagueBeast
{
    SPELL_PB_MUTATED_SPORE  = 30110,
    SPELL_PB_TRAMPLE        = 5568
};

struct mob_plagued_beastAI : public ScriptedAI
{
    mob_plagued_beastAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 MutatedSporeTimer;
    uint32 TrampleTimer;

    void Reset()
    {
        MutatedSporeTimer = 1000;
        TrampleTimer      = 3000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (MutatedSporeTimer)
        {
            if(MutatedSporeTimer <= diff)
            {
                AddSpellToCast(SPELL_PB_MUTATED_SPORE, CAST_SELF);
                MutatedSporeTimer = 0;
            }
            else
                MutatedSporeTimer -= diff;
        }

        if(TrampleTimer < diff)
        {
            AddSpellToCast(SPELL_PB_TRAMPLE, CAST_TANK);
            TrampleTimer = 3000;
        }
        else
            TrampleTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_plagued_beast(Creature *_Creature)
{
    return new mob_plagued_beastAI(_Creature);
}

/****************
* Eye Stalk - id 16236
*****************/

enum EyeStalk
{
    SPELL_ES_MIND_FLAY  = 29407
};

struct mob_eye_stalkAI : public Scripted_NoMovementAI
{
    mob_eye_stalkAI(Creature *c) : Scripted_NoMovementAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 MindFlayTimer;

    void Reset()
    {
        MindFlayTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(MindFlayTimer < diff)
        {
            AddSpellToCast(SPELL_ES_MIND_FLAY, CAST_TANK);
            MindFlayTimer = urand(1000, 10000);
        }
        else
            MindFlayTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_eye_stalk(Creature *_Creature)
{
    return new mob_eye_stalkAI(_Creature);
}

/****************
* Rotting Maggot - id 16057
*****************/

struct mob_rotting_maggotAI : public ScriptedAI
{
    mob_rotting_maggotAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    void Reset()
    {
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_rotting_maggot(Creature *_Creature)
{
    return new mob_rotting_maggotAI(_Creature);
}

/****************
* Plagued Ghoul - id 16447
*****************/

enum PlaguedGhoul
{
    SPELL_PG_FLESH_ROT  = 29915,
    SPELL_PG_REND       = 13738,
    SPELL_PG_ENRAGE     = 24318
};

struct mob_plagued_ghoulAI : public ScriptedAI
{
    mob_plagued_ghoulAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 FleshRotTimer;
    uint32 RendTimer;
    bool Enraged;

    void Reset()
    {
        FleshRotTimer = 2000;
        RendTimer     = 3000;
        Enraged       = false;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(FleshRotTimer < diff)
        {
            AddSpellToCast(SPELL_PG_FLESH_ROT, CAST_TANK);
            FleshRotTimer = urand(2000, 12000);
        }
        else
            FleshRotTimer -= diff;

        if(RendTimer < diff)
        {
            AddSpellToCast(SPELL_PG_REND, CAST_TANK);
            RendTimer = urand(3000, 15000);
        }
        else
            RendTimer -= diff;

        if(HealthBelowPct(30) && !Enraged)
        {
            AddSpellToCast(SPELL_PG_ENRAGE, CAST_SELF);
            Enraged = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_plagued_ghoul(Creature *_Creature)
{
    return new mob_plagued_ghoulAI(_Creature);
}

/*###########
# Deathknight Wing
###########*/

/****************
* Risen Deathknight - id 16154
*****************/

enum RisenDeathknight
{
    SPELL_RD_PIERCE_ARMOR  = 6016 // Release: i'm not sure it's right, but let it be
};

struct mob_risen_deathknightAI : public ScriptedAI
{
    mob_risen_deathknightAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 AggroDropTimer;
    uint32 PierceArmorTimer;

    void Reset()
    {
        AggroDropTimer = 5000; // Release: this timer need to be adjusted
        PierceArmorTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(PierceArmorTimer < diff)
        {
            AddSpellToCast(SPELL_RD_PIERCE_ARMOR, CAST_TANK);
            PierceArmorTimer = 20000;
        }
        else
            PierceArmorTimer -= diff;

        if(AggroDropTimer < diff)
        {
            DoResetThreat();
            Unit* RandomTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if(RandomTarget)
            {
                AttackStart(RandomTarget);
                me->AddThreat(RandomTarget, 1000.0f);
            }
            AggroDropTimer = 5000;
        }
        else
            AggroDropTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_risen_deathknight(Creature *_Creature)
{
    return new mob_risen_deathknightAI(_Creature);
}

/****************
* Deathknight - id 16146
*****************/

enum DeathKnight
{
    SPELL_DK_VEIL_OF_SHADOW     = 28350,
    SPELL_DK_FRIGHTENING_SHOUT  = 19134
};

struct mob_deathknightAI : public ScriptedAI
{
    mob_deathknightAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 VeilOfShadowTimer;
    uint32 FrighteningTimer;

    void Reset()
    {
        VeilOfShadowTimer = 7000;
        FrighteningTimer = 8000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(VeilOfShadowTimer < diff)
        {
            AddSpellToCast(SPELL_DK_VEIL_OF_SHADOW, CAST_TANK);
            VeilOfShadowTimer = 7000;
        }
        else
            VeilOfShadowTimer -= diff;

        if(FrighteningTimer < diff)
        {
            AddSpellToCast(SPELL_DK_FRIGHTENING_SHOUT, CAST_TANK);
            FrighteningTimer = 8000;
        }
        else
            FrighteningTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_deathknight(Creature *_Creature)
{
    return new mob_deathknightAI(_Creature);
}

/****************
* Deathknight Captain - id 16145
*****************/

enum DeathKnightCaptain
{
    SPELL_DKC_WHIRLDWIND = 28333
};

struct mob_deathknight_captainAI : public ScriptedAI
{
    mob_deathknight_captainAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 WhirldwindTimer;

    void Reset()
    {
        WhirldwindTimer = 3000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(WhirldwindTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_WHIRLDWIND, CAST_NULL);
            WhirldwindTimer = urand(7000, 14000); // Release: adjust timer
        }
        else
            WhirldwindTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_deathknight_captain(Creature *_Creature)
{
    return new mob_deathknight_captainAI(_Creature);
}

/****************
* Necro Knight - id 16165
* Release: adjust timers
*****************/

enum NecroKnight
{
    // Arcane
    SPELL_NK_BLINK              = 28391,
    SPELL_NK_ARCANE_EXPLOSION   = 15453,
    // Fire
    SPELL_NK_FIRESTRIKE         = 41379,
    SPELL_NK_BLAST_WAVE         = 30092,
    // Frost
    SPELL_NK_CONE_OF_COLD       = 30095,
    SPELL_NK_FROST_NOVA         = 30094,
    // Spec list
    SPEC_NK_ARCANE              = 1,
    SPEC_NK_FIRE                = 2,
    SPEC_NK_FROST               = 3
};

struct mob_necro_knightAI : public ScriptedAI
{
    mob_necro_knightAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 SelectSpecTimer;
    uint32 BlinkTimer;
    uint32 ArcaneExplosionTimer;
    uint32 FirestrikerTimer;
    uint32 BlastWaveTimer;
    uint32 ConeOfColdTimer;
    uint32 FrostNovaTimer;
    uint32 SpecID;

    bool RespecChecked;
    bool IsArcane;
    bool IsFire;
    bool IsFrost;

    void Reset()
    {
        SelectSpecTimer = 1000;
        BlinkTimer = 0;
        ArcaneExplosionTimer = 0;
        FirestrikerTimer = 0;
        BlastWaveTimer = 0;
        ConeOfColdTimer = 0;
        FrostNovaTimer = 0;
        RespecChecked = false;
        IsArcane = false;
        IsFire = false;
        IsFrost = false;
        SpecID = RAND(SPEC_NK_ARCANE, SPEC_NK_FIRE, SPEC_NK_FROST);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(SelectSpecTimer)
        {
            if(SelectSpecTimer < diff)
            {
                switch(SpecID)
                {
                    case SPEC_NK_ARCANE:
                    {
                        if(!IsArcane)
                        {
                            BlinkTimer = 7000;
                            ArcaneExplosionTimer = 3000;
                            IsArcane = true;
                            IsFire = false;
                            IsFrost = false;
                        }
                        break;
                    }
                    case SPEC_NK_FIRE:
                    {
                        if(!IsFire)
                        {
                            FirestrikerTimer = 5000;
                            BlastWaveTimer = 2000;
                            IsFire = true;
                            IsArcane = false;
                            IsFrost = false;
                        }
                        break;
                    }
                    case SPEC_NK_FROST:
                    {
                        if(!IsFrost)
                        {
                            ConeOfColdTimer = 2000;
                            FrostNovaTimer = 8000;
                            IsFrost = true;
                            IsArcane = false;
                            IsFire = false;
                        }
                        break;
                    }
                }
                SelectSpecTimer = 0;
            }
            else
                SelectSpecTimer -= diff;
        }

        if(HealthBelowPct(50) && !RespecChecked)
        {
            RespecChecked = true;
            switch(urand(0, 1))
            {
                case 0: // No respec
                    break;
                case 1: // Have respec
                {
                    if(IsArcane)
                    {
                        SpecID = RAND(SPEC_NK_FIRE, SPEC_NK_FROST);
                        SelectSpecTimer = 1000;
                    }
                    if(IsFire)
                    {
                        SpecID = RAND(SPEC_NK_ARCANE, SPEC_NK_FROST);
                        SelectSpecTimer = 1000;
                    }
                    if(IsFrost)
                    {
                        SpecID = RAND(SPEC_NK_ARCANE, SPEC_NK_FIRE);
                        SelectSpecTimer = 1000;
                    }
                    break;
                }
            }
        }

        if(IsArcane)
        {
            if(BlinkTimer)
            {
                if(BlinkTimer < diff)
                {
                    AddSpellToCast(SPELL_NK_BLINK, CAST_SELF);
                    BlinkTimer = urand(7000, 10000);
                } else BlinkTimer -= diff;
            }

            if(ArcaneExplosionTimer)
            {
                if(ArcaneExplosionTimer < diff)
                {
                    AddSpellToCast(SPELL_NK_ARCANE_EXPLOSION, CAST_NULL);
                    ArcaneExplosionTimer = urand(5000, 10000);
                } else ArcaneExplosionTimer -= diff;
            }
        }
        
        if(IsFire)
        {
            if(FirestrikerTimer)
            {
                if(FirestrikerTimer < diff)
                {
                    AddSpellToCast(SPELL_NK_FIRESTRIKE, CAST_NULL);
                    FirestrikerTimer = urand(5000, 15000);
                } else FirestrikerTimer -= diff;
            }

            if(BlastWaveTimer)
            {
                if(BlastWaveTimer < diff)
                {
                    AddSpellToCast(SPELL_NK_BLAST_WAVE, CAST_NULL);
                    BlastWaveTimer = urand(5000, 10000);
                } else BlastWaveTimer -= diff;
            }
        }

        if(IsFrost)
        {
            if(ConeOfColdTimer)
            {
                if(ConeOfColdTimer < diff)
                {
                    AddSpellToCast(SPELL_NK_CONE_OF_COLD, CAST_NULL);
                    ConeOfColdTimer = urand(10000, 14000);
                } else ConeOfColdTimer -= diff;
            }

            if(FrostNovaTimer)
            {
                if(FrostNovaTimer < diff)
                {
                    AddSpellToCast(SPELL_NK_FROST_NOVA, CAST_NULL);
                    FrostNovaTimer = urand(8000, 15000);
                } else FrostNovaTimer -= diff;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_necro_knight(Creature *_Creature)
{
    return new mob_necro_knightAI(_Creature);
}

/****************
* Shade of Naxxramas - id 16164
*****************/

enum ShadeofNaxxramas
{
    SPELL_SON_PORTAL_OF_SHADOWS         = 28383,
    SPELL_SON_SHADOWBOLT_VALLEY         = 28407,
    SPELL_SON_SUMMON_GHOST_OF_NAXXRAMAS = 28389
};

struct mob_shade_of_naxxramasAI : public ScriptedAI
{
    mob_shade_of_naxxramasAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 ShadowBoltValleyTimer;

    void Reset()
    {
        ShadowBoltValleyTimer = 4000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        DoCast(me, SPELL_SON_PORTAL_OF_SHADOWS, true);
    }
    
    void JustDied()
    {
        DoCast(me, SPELL_SON_SUMMON_GHOST_OF_NAXXRAMAS, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ShadowBoltValleyTimer < diff)
        {
            AddSpellToCast(SPELL_SON_PORTAL_OF_SHADOWS, CAST_TANK);
            ShadowBoltValleyTimer = 4000;
        }
        else
            ShadowBoltValleyTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shade_of_naxxramas(Creature *_Creature)
{
    return new mob_shade_of_naxxramasAI(_Creature);
}

/****************
* Portal of Shadows - id 16420
* NOTE: This is for NPC 16164
*****************/

enum PortalOfShadows
{
    SPELL_SUMMON_GHOST_OF_NAXXRAMAS = 28389
};

struct mob_portal_of_shadowsAI : public ScriptedAI
{
    mob_portal_of_shadowsAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 SummonGhostOfNaxxramasTimer;

    void Reset()
    {
        SummonGhostOfNaxxramasTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(SummonGhostOfNaxxramasTimer < diff)
        {
            AddSpellToCast(SPELL_SON_PORTAL_OF_SHADOWS, CAST_SELF);
            SummonGhostOfNaxxramasTimer = 15000;
        }
        else
            SummonGhostOfNaxxramasTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_portal_of_shadows(Creature *_Creature)
{
    return new mob_portal_of_shadowsAI(_Creature);
}

/****************
* Ghost of Naxxramas - id 16419
* NOTE: This is for NPC 16164 - 16420
*****************/

struct mob_ghost_of_naxxramasAI : public ScriptedAI
{
    mob_ghost_of_naxxramasAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 CheckTimer;

    void Reset()
    {
        CheckTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CheckTimer)
        {
            if(CheckTimer < diff)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                    me->Attack(target, true);
                CheckTimer = 0;
            }
            else
                CheckTimer -= diff;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ghost_of_naxxramas(Creature *_Creature)
{
    return new mob_ghost_of_naxxramasAI(_Creature);
}

/****************
* Dark Touched Warrior - id 16156
*****************/

struct mob_dark_touched_warriorAI : public ScriptedAI
{
    mob_dark_touched_warriorAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 AggroWipeTimer;

    void Reset()
    {
        AggroWipeTimer = 5000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(AggroWipeTimer < diff)
        {
            DoResetThreat();
            AggroWipeTimer = urand(4000, 7000);
        }
        else
            AggroWipeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_dark_touched_warrior(Creature *_Creature)
{
    return new mob_dark_touched_warriorAI(_Creature);
}

/****************
* Death Touched Warrior - id 16158
*****************/

struct mob_death_touched_warriorAI : public ScriptedAI
{
    mob_death_touched_warriorAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 AggroWipeTimer;

    void Reset()
    {
        AggroWipeTimer = 5000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(AggroWipeTimer < diff)
        {
            DoResetThreat();
            AggroWipeTimer = urand(4000, 7000);
        }
        else
            AggroWipeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_death_touched_warrior(Creature *_Creature)
{
    return new mob_death_touched_warriorAI(_Creature);
}

/****************
* Skeletal Smith - id 16193
*****************/

enum SkeletalSmith
{
    SPELL_SS_CRUSH_ARMOR    = 33661,
    SPELL_SS_DISARM         = 6713
};

struct mob_skeletal_smithAI : public ScriptedAI
{
    mob_skeletal_smithAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 DisarmTimer;
    uint32 CrushArmorTimer;

    void Reset()
    {
        DisarmTimer     = 6000;
        CrushArmorTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CrushArmorTimer < diff)
        {
            AddSpellToCast(SPELL_SS_CRUSH_ARMOR, CAST_TANK);
            CrushArmorTimer = urand(1000, 2000);
        }
        else
            CrushArmorTimer -= diff;

        if(DisarmTimer < diff)
        {
            AddSpellToCast(SPELL_SS_DISARM, CAST_RANDOM);
            DisarmTimer = 6000;
        }
        else
            DisarmTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_skeletal_smith(Creature *_Creature)
{
    return new mob_skeletal_smithAI(_Creature);
}

/****************
* Bony Construct - id 16167
*****************/

enum BonyConstruct
{
    SPELL_BC_CLEAVE         = 19632,
    SPELL_BC_SWEEPING_SLAM  = 25322
};

struct mob_bony_constructAI : public ScriptedAI
{
    mob_bony_constructAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 CleaveTimer;
    uint32 SweepingSlamTimer;

    void Reset()
    {
        CleaveTimer       = 3000;
        SweepingSlamTimer = 7000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_BC_CLEAVE, CAST_TANK);
            CleaveTimer = urand(3000, 4000);
        }
        else
            CleaveTimer -= diff;

        if(SweepingSlamTimer < diff)
        {
            AddSpellToCast(SPELL_BC_SWEEPING_SLAM, CAST_TANK);
            SweepingSlamTimer = 7000;
        }
        else
            SweepingSlamTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bony_construct(Creature *_Creature)
{
    return new mob_bony_constructAI(_Creature);
}

/****************
* Deathknight Cavalier - id 16163
*****************/

enum DeathknightCavalier
{
    SPELL_DKC_CLEAVE        = 15284,
    SPELL_DKC_DEATH_COIL    = 28412,
    SPELL_DKC_AURA_OF_AGONY = 28413
};

struct mob_deathknight_cavalierAI : public ScriptedAI
{
    mob_deathknight_cavalierAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 DeathCoilTimer;
    uint32 CleaveTimer;
    uint32 AuraOfAgonyTimer;

    void Reset()
    {
        CleaveTimer         = 1000;
        DeathCoilTimer      = 3000;
        AuraOfAgonyTimer    = 8000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_CLEAVE, CAST_TANK);
            CleaveTimer = urand(1000, 4000);
        }
        else
            CleaveTimer -= diff;

        if(DeathCoilTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_DEATH_COIL, CAST_TANK);
            DeathCoilTimer = urand(3000, 5000);
        }
        else
            DeathCoilTimer -= diff;

        if(AuraOfAgonyTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_AURA_OF_AGONY, CAST_RANDOM);
            AuraOfAgonyTimer = 8000;
        }
        else
            AuraOfAgonyTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_deathknight_cavalier(Creature *_Creature)
{
    return new mob_deathknight_cavalierAI(_Creature);
}

/****************
* Skeletal Steed - id 16067
*****************/

enum SkeletalSteed
{
    SPELL_SS_TRAMPLE    = 5568,
    SPELL_SS_INTERCEPT  = 27577
};

struct mob_skeletal_steedAI : public ScriptedAI
{
    mob_skeletal_steedAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 TrampleTimer;
    uint32 InterceptTimer;

    void Reset()
    {
        TrampleTimer    = 2000;
        InterceptTimer  = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(TrampleTimer < diff)
        {
            AddSpellToCast(SPELL_SS_TRAMPLE, CAST_TANK);
            TrampleTimer = urand(1000, 4000);
        }
        else
            TrampleTimer -= diff;

        if(InterceptTimer < diff)
        {
            AddSpellToCast(SPELL_SS_INTERCEPT, CAST_RANDOM);
            InterceptTimer = urand(5000, 7000);
        }
        else
            InterceptTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_skeletal_steed(Creature *_Creature)
{
    return new mob_skeletal_steedAI(_Creature);
}

/****************
* Doom Touched Warrior - id 16157
*****************/

enum DoomTouchedWarrior
{
    SPELL_DTW_TRAMPLE    = 5568
};

struct mob_doom_touched_warriorAI : public ScriptedAI
{
    mob_doom_touched_warriorAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

   uint32 AggroWipeTimer;

    void Reset()
    {
        AggroWipeTimer = 5000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(AggroWipeTimer < diff)
        {
            DoResetThreat();
            AggroWipeTimer = urand(4000, 7000);
        }
        else
            AggroWipeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_doom_touched_warrior(Creature *_Creature)
{
    return new mob_doom_touched_warriorAI(_Creature);
}

/*###########
# Tunnel
###########*/

/****************
* Necropolis Acolyte - id 16368
*****************/

struct mob_necropolis_acolyteAI : public ScriptedAI
{
    mob_necropolis_acolyteAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 ArcaneExplosionTimer;
    uint32 ShadowboltVolleyTimer;

    void Reset()
    {
        ClearCastQueue();
        ArcaneExplosionTimer = 2000;
        ShadowboltVolleyTimer = 4000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ArcaneExplosionTimer < diff)
        {
            AddSpellToCast(SPELL_NA_ARCANE_EXPLOSION, CAST_NULL);
            ArcaneExplosionTimer = urand(2000, 5000);
        }
        else
            ArcaneExplosionTimer -= diff;

        if(ShadowboltVolleyTimer < diff)
        {
            AddSpellToCast(SPELL_NA_SHADOWBOLT_VOLLEY, CAST_TANK);
            ShadowboltVolleyTimer = urand(4000, 6000);
        }
        else
            ShadowboltVolleyTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_necropolis_acolyte(Creature *_Creature)
{
    return new mob_necropolis_acolyteAI(_Creature);
}

/****************
* Necro Knight Guardian - id 16452
* Release: adjust timers
*****************/

enum NecroKnightGuardian
{
    // Arcane
    SPELL_NKG_BLINK              = 28391,
    SPELL_NKG_ARCANE_EXPLOSION   = 15453,
    // Fire
    SPELL_NKG_FIRESTRIKE         = 41379,
    SPELL_NKG_BLAST_WAVE         = 30092,
    // Frost
    SPELL_NKG_CONE_OF_COLD       = 30095,
    SPELL_NKG_FROST_NOVA         = 30094,
    // Spec list
    SPEC_NKG_ARCANE              = 1,
    SPEC_NKG_FIRE                = 2,
    SPEC_NKG_FROST               = 3
};

struct mob_necro_knight_guardianAI : public ScriptedAI
{
    mob_necro_knight_guardianAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 SelectSpecTimer;
    uint32 BlinkTimer;
    uint32 ArcaneExplosionTimer;
    uint32 FirestrikerTimer;
    uint32 BlastWaveTimer;
    uint32 ConeOfColdTimer;
    uint32 FrostNovaTimer;
    uint32 SpecID;

    bool RespecChecked;
    bool IsArcane;
    bool IsFire;
    bool IsFrost;

    void Reset()
    {
        SelectSpecTimer = 1000;
        BlinkTimer = 0;
        ArcaneExplosionTimer = 0;
        FirestrikerTimer = 0;
        BlastWaveTimer = 0;
        ConeOfColdTimer = 0;
        FrostNovaTimer = 0;
        RespecChecked = false;
        IsArcane = false;
        IsFire = false;
        IsFrost = false;
        SpecID = RAND(SPEC_NKG_ARCANE, SPEC_NKG_FIRE, SPEC_NKG_FROST);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(SelectSpecTimer)
        {
            if(SelectSpecTimer < diff)
            {
                switch(SpecID)
                {
                    case SPEC_NKG_ARCANE:
                    {
                        if(!IsArcane)
                        {
                            BlinkTimer = 7000;
                            ArcaneExplosionTimer = 3000;
                            IsArcane = true;
                            IsFire = false;
                            IsFrost = false;
                        }
                        break;
                    }
                    case SPEC_NKG_FIRE:
                    {
                        if(!IsFire)
                        {
                            FirestrikerTimer = 5000;
                            BlastWaveTimer = 2000;
                            IsFire = true;
                            IsArcane = false;
                            IsFrost = false;
                        }
                        break;
                    }
                    case SPEC_NKG_FROST:
                    {
                        if(!IsFrost)
                        {
                            ConeOfColdTimer = 2000;
                            FrostNovaTimer = 8000;
                            IsFrost = true;
                            IsArcane = false;
                            IsFire = false;
                        }
                        break;
                    }
                }
                SelectSpecTimer = 0;
            }
            else
                SelectSpecTimer -= diff;
        }

        if(HealthBelowPct(50) && !RespecChecked)
        {
            RespecChecked = true;
            switch(urand(0, 1))
            {
                case 0: // No respec
                    break;
                case 1: // Have respec
                {
                    if(IsArcane)
                    {
                        SpecID = RAND(SPEC_NKG_FIRE, SPEC_NKG_FROST);
                        SelectSpecTimer = 1000;
                    }
                    if(IsFire)
                    {
                        SpecID = RAND(SPEC_NKG_ARCANE, SPEC_NKG_FROST);
                        SelectSpecTimer = 1000;
                    }
                    if(IsFrost)
                    {
                        SpecID = RAND(SPEC_NKG_ARCANE, SPEC_NKG_FIRE);
                        SelectSpecTimer = 1000;
                    }
                    break;
                }
            }
        }

        if(IsArcane)
        {
            if(BlinkTimer)
            {
                if(BlinkTimer < diff)
                {
                    AddSpellToCast(SPELL_NKG_BLINK, CAST_SELF);
                    BlinkTimer = urand(7000, 10000);
                } else BlinkTimer -= diff;
            }

            if(ArcaneExplosionTimer)
            {
                if(ArcaneExplosionTimer < diff)
                {
                    AddSpellToCast(SPELL_NKG_ARCANE_EXPLOSION, CAST_NULL);
                    ArcaneExplosionTimer = urand(5000, 10000);
                } else ArcaneExplosionTimer -= diff;
            }
        }
        
        if(IsFire)
        {
            if(FirestrikerTimer)
            {
                if(FirestrikerTimer < diff)
                {
                    AddSpellToCast(SPELL_NKG_FIRESTRIKE, CAST_NULL);
                    FirestrikerTimer = urand(5000, 15000);
                } else FirestrikerTimer -= diff;
            }

            if(BlastWaveTimer)
            {
                if(BlastWaveTimer < diff)
                {
                    AddSpellToCast(SPELL_NKG_BLAST_WAVE, CAST_NULL);
                    BlastWaveTimer = urand(5000, 10000);
                } else BlastWaveTimer -= diff;
            }
        }

        if(IsFrost)
        {
            if(ConeOfColdTimer)
            {
                if(ConeOfColdTimer < diff)
                {
                    AddSpellToCast(SPELL_NKG_CONE_OF_COLD, CAST_NULL);
                    ConeOfColdTimer = urand(10000, 14000);
                } else ConeOfColdTimer -= diff;
            }

            if(FrostNovaTimer)
            {
                if(FrostNovaTimer < diff)
                {
                    AddSpellToCast(SPELL_NKG_FROST_NOVA, CAST_NULL);
                    FrostNovaTimer = urand(8000, 15000);
                } else FrostNovaTimer -= diff;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_necro_knight_guardian(Creature *_Creature)
{
    return new mob_necro_knight_guardianAI(_Creature);
}

/****************
* Deathknight Vindicator - id 16451
*****************/

struct mob_deathknight_vindicatorAI : public ScriptedAI
{
    mob_deathknight_vindicatorAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 DeathCoilTimer;
    uint32 CleaveTimer;
    uint32 AuraOfAgonyTimer;

    void Reset()
    {
        CleaveTimer         = 1000;
        DeathCoilTimer      = 3000;
        AuraOfAgonyTimer    = 8000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_CLEAVE, CAST_TANK);
            CleaveTimer = urand(1000, 4000);
        }
        else
            CleaveTimer -= diff;

        if(DeathCoilTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_DEATH_COIL, CAST_TANK);
            DeathCoilTimer = urand(3000, 5000);
        }
        else
            DeathCoilTimer -= diff;

        if(AuraOfAgonyTimer < diff)
        {
            AddSpellToCast(SPELL_DKC_AURA_OF_AGONY, CAST_RANDOM);
            AuraOfAgonyTimer = 8000;
        }
        else
            AuraOfAgonyTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_deathknight_vindicator(Creature *_Creature)
{
    return new mob_deathknight_vindicatorAI(_Creature);
}

void AddSC_trash_naxxramas()
{
    Script *newscript;

    // Abdomination Wing
    newscript = new Script;
    newscript->Name = "mob_16017";
    newscript->GetAI = &GetAI_mob_patchwork_golem;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16018";
    newscript->GetAI = &GetAI_mob_bile_retcher;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16375";
    newscript->GetAI = &GetAI_mob_sewage_slime;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16029";
    newscript->GetAI = &GetAI_mob_sludge_belcher;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16024";
    newscript->GetAI = &GetAI_mob_embalming_slime;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16021";
    newscript->GetAI = &GetAI_mob_living_monstrosity;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16022";
    newscript->GetAI = &GetAI_mob_surgical_assistant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16020";
    newscript->GetAI = &GetAI_mob_mad_scientist;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "mob_16025";
    newscript->GetAI = &GetAI_mob_stitched_spewer;
    newscript->RegisterSelf();
    
    // Spider Wing
    newscript = new Script;
    newscript->Name = "mob_15977";
    newscript->GetAI = &GetAI_mob_infectious_skitterer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_15975";
    newscript->GetAI = &GetAI_mob_carrion_spinner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_15974";
    newscript->GetAI = &GetAI_mob_dread_creeper;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "mob_15976";
    newscript->GetAI = &GetAI_mob_venom_stalker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_15978";
    newscript->GetAI = &GetAI_mob_crypt_reaver;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16453";
    newscript->GetAI = &GetAI_mob_necro_stalker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_15980";
    newscript->GetAI = &GetAI_mob_naxxramas_cultist;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_15981";
    newscript->GetAI = &GetAI_mob_naxxramas_acolyte;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16573";
    newscript->GetAI = &GetAI_mob_crypt_guard;
    newscript->RegisterSelf();

    // Plague Wing
    newscript = new Script;
    newscript->Name = "mob_16244";
    newscript->GetAI = &GetAI_mob_infectious_ghoul;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16243";
    newscript->GetAI = &GetAI_mob_plague_slime;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16168";
    newscript->GetAI = &GetAI_mob_stoneskin_gargoyle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16297";
    newscript->GetAI = &GetAI_mob_mutated_grub;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16037";
    newscript->GetAI = &GetAI_mob_plagued_bat;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16034";
    newscript->GetAI = &GetAI_mob_plagued_beast;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16236";
    newscript->GetAI = &GetAI_mob_eye_stalk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16057";
    newscript->GetAI = &GetAI_mob_rotting_maggot;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16447";
    newscript->GetAI = &GetAI_mob_plagued_ghoul;
    newscript->RegisterSelf();

    // Deathknight Wing
    newscript = new Script;
    newscript->Name = "mob_16154";
    newscript->GetAI = &GetAI_mob_risen_deathknight;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16146";
    newscript->GetAI = &GetAI_mob_deathknight;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16145";
    newscript->GetAI = &GetAI_mob_deathknight_captain;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16165";
    newscript->GetAI = &GetAI_mob_necro_knight;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16164";
    newscript->GetAI = &GetAI_mob_shade_of_naxxramas;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "mob_16420";
    newscript->GetAI = &GetAI_mob_portal_of_shadows;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16419";
    newscript->GetAI = &GetAI_mob_ghost_of_naxxramas;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "mob_16158";
    newscript->GetAI = &GetAI_mob_death_touched_warrior;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16193";
    newscript->GetAI = &GetAI_mob_skeletal_smith;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16167";
    newscript->GetAI = &GetAI_mob_bony_construct;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16163";
    newscript->GetAI = &GetAI_mob_deathknight_cavalier;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16067";
    newscript->GetAI = &GetAI_mob_skeletal_steed;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16157";
    newscript->GetAI = &GetAI_mob_doom_touched_warrior;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "mob_16368";
    newscript->GetAI = &GetAI_mob_necropolis_acolyte;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16452";
    newscript->GetAI = &GetAI_mob_necro_knight_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_16451";
    newscript->GetAI = &GetAI_mob_deathknight_vindicator;
    newscript->RegisterSelf();
}