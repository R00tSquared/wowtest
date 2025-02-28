/* ScriptData
SDName: trash_botanica
SD%Complete:
SDComment: Trash NPC
SDCategory: The Botanica
EndScriptData */

#include "precompiled.h"

/****************
* Mutate Fleshlasher - N: 19598 H: 21561
*****************/

enum MutateFleshlasher
{
    SPELL_MF_VICIOUS_BITE   = 34351
};

struct mob_mutate_fleshlasherAI : public ScriptedAI
{
    mob_mutate_fleshlasherAI(Creature *c) : ScriptedAI(c) { }

    Timer ViciousBiteTimer;

    void Reset()
    {
        ClearCastQueue();
        ViciousBiteTimer.Reset(urand(7800, 12100));
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ViciousBiteTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_MF_VICIOUS_BITE, CAST_TANK);
            ViciousBiteTimer = urand(6200, 12100);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mutate_fleshlasherAI(Creature *_Creature)
{
    return new mob_mutate_fleshlasherAI(_Creature);
}

/****************
* Sunseeker Geomancer - N: 18420 H: 21574
*****************/

enum SunseekerGeomancer
{
    SPELL_SG_BLIZZARD                   = 34167,
    SPELL_SG_RAIN_OF_FIRE               = 34169,
    SPELL_SG_ARCANE_EXPLOSION           = 34170,
    SPELL_SG_BLIZZARD2                  = 34183,
    SPELL_SG_RAIN_OF_FIRE2              = 34185,
    SPELL_SG_ARCANE_EXPLOSION_COMBAT    = 35124,
    SPELL_SG_FIRE_SHIELD                = 35265
};

struct mob_sunseeker_geomancerAI : public ScriptedAI
{
    mob_sunseeker_geomancerAI(Creature *c) : ScriptedAI(c) { HeroicMode = m_creature->GetMap()->IsHeroic(); }

    Timer BlizzardChanneling_Timer;
    Timer RainOfFire_Timer;
    Timer ArcaneExplosion_Timer;
    Timer Explosion_Timer;
    bool HeroicMode;

    void Reset()
    {
        ClearCastQueue();
        BlizzardChanneling_Timer.Reset(1000);
        RainOfFire_Timer.Reset(30000);
        ArcaneExplosion_Timer.Reset(20000);
        Explosion_Timer.Reset(1000);
        // me->GetMotionMaster()->MoveRandomAroundPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 10.0f);
        me->RemoveAurasDueToSpell(SPELL_SG_FIRE_SHIELD);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        me->CastSpell(me, SPELL_SG_FIRE_SHIELD, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(BlizzardChanneling_Timer.Expired(diff))
            {
                DoCastAOE(HeroicMode ? SPELL_SG_BLIZZARD2 : SPELL_SG_BLIZZARD, false);
                BlizzardChanneling_Timer = 30000;
            }
            if(RainOfFire_Timer.Expired(diff))
            {
                DoCastAOE(HeroicMode ? SPELL_SG_RAIN_OF_FIRE2 : SPELL_SG_RAIN_OF_FIRE, false);
                RainOfFire_Timer = 30000;
            }
            if(ArcaneExplosion_Timer.Expired(diff))
            {
                DoCastAOE(SPELL_SG_ARCANE_EXPLOSION, false);
                ArcaneExplosion_Timer = 30000;
            }
            return;
        }

        if (Explosion_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_SG_ARCANE_EXPLOSION_COMBAT, CAST_TANK);
            Explosion_Timer = urand(3600, 8400);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_sunseeker_geomancerAI(Creature *_Creature)
{
    return new mob_sunseeker_geomancerAI(_Creature);
}

/****************
* Sunseeker Channeler - N: 19505 H: 21571
*****************/

enum SunseekerChanneler
{
    SPELL_SC_SOUL_CHANNEL                   = 34637,
    SPELL_SC_SUNSEEKER_BLESSING_HEAL        = 34222,
    SPELL_SC_CRYSTAL_CHANNEL_PER_30         = 34156,
    NPC_CHANNELING_TARGET                   = 32003
};

struct mob_sunseeker_channelerAI : public ScriptedAI
{
    mob_sunseeker_channelerAI(Creature *c) : ScriptedAI(c) { }

    Timer Movement_Timer;
    Timer Movement2_Timer;
    Timer Channeling_Timer;

    void Reset()
    {
        ClearCastQueue();
        Movement_Timer.Reset(0);
        Movement2_Timer.Reset(0);
        Channeling_Timer.Reset(0);
        switch(me->GetDBTableGUIDLow())
        {
            case 82988:
                me->CastSpell(me, SPELL_SC_SUNSEEKER_BLESSING_HEAL, false);
                me->GetMotionMaster()->MovePoint(1, 21.516, 588.565, -17.763);
                break;
            case 83077:
                me->CastSpell(me, SPELL_SC_SUNSEEKER_BLESSING_HEAL, false);
                me->GetMotionMaster()->MovePoint(1, -11.53, 590.83, -17.707);
                break;
            default:
                Channeling_Timer = 1000;
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 1)
        {
            Channeling_Timer = 1000;
            Movement_Timer = 15000;
        }
        else if (id == 2)
        {
            if(Unit* Thorngrin = FindCreature(17978, 30.0f, me))
                me->SetFacingToObject(Thorngrin);
            me->CastSpell(me, SPELL_SC_SUNSEEKER_BLESSING_HEAL, false);
            Movement2_Timer = 10000;
        }
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(Movement_Timer.Expired(diff))
            {
                switch(me->GetDBTableGUIDLow())
                {
                    case 82988:
                        me->GetMotionMaster()->MovePoint(2, 19.331, 574.729, -17.948);
                        break;
                    case 83077:
                        me->GetMotionMaster()->MovePoint(2, -4.02, 576.775, -17.876);
                        break;
                    default: break;
                }
                Movement_Timer = 0;
            }

            if(Movement2_Timer.Expired(diff))
            {
                switch(me->GetDBTableGUIDLow())
                {
                    case 82988:
                        Channeling_Timer = 0;
                        me->GetMotionMaster()->MovePoint(1, 21.516, 588.565, -17.763);
                        break;
                    case 83077:
                        Channeling_Timer = 0;
                        me->GetMotionMaster()->MovePoint(1, -11.53, 590.83, -17.707);
                        break;
                    default: break;
                }
                Movement2_Timer = 0;
            }

            if(Channeling_Timer.Expired(diff))
            {
                me->CastSpell((Unit*)NULL,SPELL_SC_CRYSTAL_CHANNEL_PER_30,false);
                Channeling_Timer = 35000;
            }
            return;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_sunseeker_channelerAI(Creature *_Creature)
{
    return new mob_sunseeker_channelerAI(_Creature);
}

void AddSC_trash_botanica()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_mutate_fleshlasher";
    newscript->GetAI = &GetAI_mob_mutate_fleshlasherAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_sunseeker_geomancerAI";
    newscript->GetAI = &GetAI_mob_sunseeker_geomancerAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "sunseeker_channeler";
    newscript->GetAI = &GetAI_mob_sunseeker_channelerAI;
    newscript->RegisterSelf();
}