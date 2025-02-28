// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* ScriptData
SDName: trash_hellfire_ramparts
SD%Complete:
SDComment: Trash NPC
SDCategory: Hellfire Ramparts
EndScriptData */

#include "precompiled.h"
#include "hellfire_ramparts.h"

/****************
* Bleeding Hollow Scryer - N: 17478 H: 18050
*****************/

enum BleedingHollowScryer
{
    SPELL_BHS_SHADOWBOLT_N  = 12471,
    SPELL_BHS_SHADOWBOLT_H  = 15232,
    SPELL_BHS_FEAR          = 30615,
    SPELL_BHS_FEL_INFUSION  = 30659,
    SPELL_BHS_VISUAL        = 31059,
    SAY_BHS_1               = -1901001,
    SAY_BHS_2               = -1901002,
    SAY_BHS_3               = -1901003
};

struct mob_bleeding_hollow_scryerAI : public ScriptedAI
{
    mob_bleeding_hollow_scryerAI(Creature *c) : ScriptedAI(c) { DoCast(me, SPELL_BHS_VISUAL, false); }

    Timer ShadowBoltTimer;
    Timer FearTimer;    

    void Reset()
    {
        ShadowBoltTimer.Reset(urand(1000, 2000));
        FearTimer.Reset(7000);
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void JustReachedHome()
    {
        ClearCastQueue();
        DoCast(me, SPELL_BHS_VISUAL, false);
    }

    void EnterCombat(Unit*)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(RAND(SAY_BHS_1, SAY_BHS_2, SAY_BHS_3), me); break;
            default: break;
        }
        
        me->InterruptNonMeleeSpells(true);
        DoZoneInCombat(80.0f);
    }

    void JustDied()
    {
        DoCast(me, SPELL_BHS_FEL_INFUSION, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ShadowBoltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), HeroicMode ? SPELL_BHS_SHADOWBOLT_H : SPELL_BHS_SHADOWBOLT_N);
            ShadowBoltTimer = urand(3000, 4000);
        }

        if (FearTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_BHS_FEAR);
            FearTimer = urand(7000, 8000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bleeding_hollow_scryerAI(Creature *_Creature)
{
    return new mob_bleeding_hollow_scryerAI(_Creature);
}

/****************
* Bonechewer Beastmaster - N: 17455 H: 18051
*****************/

enum BonechewerBeastmaster
{
    SPELL_BBM_BATTLESHOUT   = 9128,
    SPELL_BBM_UPPERCUT      = 10966,
    NPC_WARHOUND            = 17280,

    YELL_BBM_1               = -1901004,
    YELL_BBM_2               = -1901005,
};

struct mob_bonechewer_beastmasterAI : public ScriptedAI
{
    mob_bonechewer_beastmasterAI(Creature *c) : ScriptedAI(c), summons(c) { }

    SummonList summons;
    Timer BattleshoutTimer;
    Timer UppercutTimer;
    Timer SummonWarhoundTimer;

    void Reset()
    {
        ClearCastQueue();
        BattleshoutTimer.Reset(4000);
        UppercutTimer.Reset(2000);
        SummonWarhoundTimer.Reset(17000);
        summons.DespawnAll();
    }

    void EnterCombat(Unit*)
    {
        DoScriptText(YELL_BBM_1, me);
        DoZoneInCombat(80.0f);
    }

    void JustDied()
    {
        summons.DespawnAll();
    }

    void JustSummoned(Creature *summoned)
    {
        if(summoned)
            summons.Summon(summoned);

        if(summoned->GetEntry() == NPC_WARHOUND)
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

        if (BattleshoutTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_BBM_BATTLESHOUT);
            BattleshoutTimer = urand(15000, 30000);
        }

        if (UppercutTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_BBM_UPPERCUT);
            UppercutTimer = urand(7000, 12000);
        }
        
        if(SummonWarhoundTimer.Expired(diff))
        {
            DoScriptText(YELL_BBM_2, me);
            me->SummonCreature(NPC_WARHOUND, -1301.99, 1538.4, 68.609, 0.595, TEMPSUMMON_CORPSE_DESPAWN, 300000);
            me->SummonCreature(NPC_WARHOUND, -1294.82, 1531.5, 68.59, 0.99, TEMPSUMMON_CORPSE_DESPAWN, 300000);
            me->SummonCreature(NPC_WARHOUND, -1285.904, 1529.081, 68.57, 1.32, TEMPSUMMON_CORPSE_DESPAWN, 300000);
            SummonWarhoundTimer = 0;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_beastmasterAI(Creature *_Creature)
{
    return new mob_bonechewer_beastmasterAI(_Creature);
}

void AddSC_trash_hellfire_ramparts()
{
    Script *newscript;

    // UPDATE `creature_template` SET `AIName`='', `ScriptName`='mob_bleeding_hollow_scryer' WHERE `entry` IN (17478, 18050);
    newscript = new Script;
    newscript->Name = "mob_bleeding_hollow_scryer";
    newscript->GetAI = &GetAI_mob_bleeding_hollow_scryerAI;
    newscript->RegisterSelf();
    // UPDATE `creature_template` SET `AIName`='', `ScriptName`='mob_bonechewer_beastmaster' WHERE `entry` IN (17455, 18051);
    newscript = new Script;
    newscript->Name = "mob_bonechewer_beastmaster";
    newscript->GetAI = &GetAI_mob_bonechewer_beastmasterAI;
    newscript->RegisterSelf();
}