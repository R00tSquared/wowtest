/* ScriptData
SDName: Boss Mechano-Lord Capacitus
SD%Complete:
SDComment:
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "precompiled.h"
#include "def_mechanar.h"

enum MechanoLordCapacitus
{
    SPELL_HEAD_CRACK            = 35161,
    SPELL_REFLECTIVE_DMG_SHIELD = 35159,
    SPELL_REFLECTIVE_SPD_SHIELD = 35158,
    SPELL_POLARITY_SHIFT        = 39096,
    SPELL_BERSERK               = 26662,
    NPC_NETHER_BOMB             = 20405,
    YELL_AGGRO                  = -1901031,
    YELL_RANDOM_1               = -1901032,
    YELL_RANDOM_2               = -1901033,
    YELL_KILL_1                 = -1901034,
    YELL_KILL_2                 = -1901035,
    YELL_DEATH                  = -1901036
};

struct boss_mechanolord_capacitusAI : public ScriptedAI
{
    boss_mechanolord_capacitusAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        pInstance = c->GetInstanceData();
        HeroicMode = me->GetMap()->IsHeroic();
        me->GetPosition(wLoc);
    }

    ScriptedInstance *pInstance;

    Timer SummonTimer;
    SummonList summons;
    Timer HeadCrackTimer;
    Timer ReflectiveDmgShieldTimer;
    Timer ReflectiveSpdShieldTimer;
    Timer PolarityShiftTimer;
    Timer BerserkTimer;
    Timer Check_Timer;
    WorldLocation wLoc;
    bool HeroicMode;
    bool Enraged;

    uint32 Counter;

    void Reset()
    {
        HeadCrackTimer.Reset(urand(16100,18600));
        ReflectiveDmgShieldTimer.Reset(15000);
        ReflectiveSpdShieldTimer.Reset(35000);
        PolarityShiftTimer.Reset(urand(24500, 29700));
        BerserkTimer.Reset(180000);
        SummonTimer.Reset(HeroicMode ? urand(9000, 11000) : urand(2000, 5000));
        summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(DATA_MECHANO_LORD_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO, me);
        if(pInstance)
            pInstance->SetData(DATA_MECHANO_LORD_EVENT, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(YELL_KILL_1, YELL_KILL_2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(YELL_DEATH, me);
        summons.Cast(20405, 5, NULL);
        if(pInstance)
            pInstance->SetData(DATA_MECHANO_LORD_EVENT, DONE);
    }

    void JustSummoned(Creature *summon)
    {
        summons.Summon(summon);
        summon->GetMotionMaster()->MoveRandomAroundPoint(summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ(), 15.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(SummonTimer.Expired(diff))
        {
            float x, y, z;
            me->GetNearPoint(x, y, z, 0, 7, 0);
            me->SummonCreature(NPC_NETHER_BOMB, x, y, z, 0, TEMPSUMMON_CORPSE_DESPAWN, 1000);
            SummonTimer.Reset(HeroicMode ? urand(3000, 6000) : urand(9000, 13000));
        }

        if (HeadCrackTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_HEAD_CRACK, CAST_TANK);
            HeadCrackTimer = urand(19700, 33200);
        }

        if (Check_Timer.Expired(diff))
        {
            if(!me->IsWithinDistInMap(&wLoc, 50.0f))
            {
                Check_Timer = 0;
                EnterEvadeMode();
            }
            else
                DoZoneInCombat();
                Check_Timer = 3000;
        }

        if(!HeroicMode)
        {
            if (ReflectiveDmgShieldTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_REFLECTIVE_DMG_SHIELD, CAST_SELF);
                DoScriptText(RAND(YELL_RANDOM_1, YELL_RANDOM_2), me);
                ReflectiveDmgShieldTimer = 40000;
            }
        }

        if(!HeroicMode)
        {
            if (ReflectiveSpdShieldTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_REFLECTIVE_SPD_SHIELD, CAST_SELF);
                DoScriptText(RAND(YELL_RANDOM_1, YELL_RANDOM_2), me);
                ReflectiveSpdShieldTimer = 40000;
            }
        }
        
        if(HeroicMode)
        {
            if (PolarityShiftTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_POLARITY_SHIFT, CAST_SELF);
                DoScriptText(RAND(YELL_RANDOM_1, YELL_RANDOM_2), me);
                PolarityShiftTimer = urand(27700, 33800);
            }
        }
        
        if(HeroicMode)
        {
            if (BerserkTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_BERSERK, CAST_SELF);
                BerserkTimer = 0;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_mechanolord_capacitus(Creature *_Creature)
{
    return new boss_mechanolord_capacitusAI (_Creature);
}

void AddSC_boss_mechanolord_capacitus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_mechanolord_capacitus";
    newscript->GetAI = &GetAI_boss_mechanolord_capacitus;
    newscript->RegisterSelf();
}

