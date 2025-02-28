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
SDName: Boss_DrekThar
SD%Complete: 50%
SDComment: Some spells listed on wowwiki but doesn't exist on wowhead
EndScriptData */

#include "precompiled.h"

#define YELL_AGGRO             -2100000

#define YELL_EVADE             -2100001
#define YELL_RESPAWN           -2100002

#define YELL_RANDOM1           -2100003
#define YELL_RANDOM2           -2100004
#define YELL_RANDOM3           -2100005
#define YELL_RANDOM4           -2100006
#define YELL_RANDOM5           -2100007


#define SPELL_WHIRLWIND        15589
#define SPELL_WHIRLWIND2       13736
#define SPELL_KNOCKDOWN        19128
#define SPELL_FRENZY           8269
#define SPELL_SWEEPING_STRIKES 18765 // not sure
#define SPELL_CLEAVE           20677 // not sure
#define SPELL_WINDFURY         35886 // not sure
#define SPELL_STORMPIKE        51876 // not sure

#define AV_DREKTHAR_NPC_COUNT   5

uint32 avDrekTharNpcId[AV_DREKTHAR_NPC_COUNT] =
{
    14772,
    14777,
    14776,
    14773,
    11946
};

struct boss_drektharAI : public ScriptedAI
{
    boss_drektharAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
        m_creature->SetIsDistanceToHomeEvadable(false);
    }

    Timer_UnCheked WhirlwindTimer;
    Timer_UnCheked Whirlwind2Timer;
    Timer_UnCheked KnockdownTimer;
    Timer_UnCheked FrenzyTimer;
    Timer_UnCheked YellTimer;
    Timer_UnCheked CheckTimer;
    Timer_UnCheked CleaveTimer;
    WorldLocation wLoc;
    bool announced;

    void Reset()
    {
        WhirlwindTimer.Reset(urand(0, 7000));
        Whirlwind2Timer.Reset(urand(0, 11000));
        KnockdownTimer.Reset(9000);
        FrenzyTimer.Reset(6000);
        YellTimer.Reset(urand(20000, 30000)); //20 to 30 seconds
        CheckTimer.Reset(2000);
        CleaveTimer.Reset(12000);
        me->SetAggroRange(20.0f);
        announced = false;
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO, m_creature);

        // pull rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this, who] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->isAlive() && c->IsAIEnabled && c->AI())
                            c->AI()->AttackStart(who);
                  });
    }

    void JustRespawned()
    {
        Reset();
        DoScriptText(YELL_RESPAWN, m_creature);
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        // evade rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->IsInCombat() && c->IsAIEnabled && c->AI())
                            c->AI()->EnterEvadeMode();
                  });
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!announced && me->GetHealthPercent() < 90)
        {
            Unit* unit = me->GetVictim();
            if (unit)
            {
                Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (player)
                {
                    BattleGround* bg = player->GetBattleGround();
                    if (bg)
                    {
                        bg->SendNotifyToTeam(HORDE, 16733);
                        bg->PlaySoundToTeam(10843, HORDE);
                        announced = true;
                    }
                }
            }
        }

        if (CheckTimer.Expired(diff))
        {
            if(!m_creature->IsWithinDistInMap(&wLoc, 50.0f))
                EnterEvadeMode();

            me->SetSpeed(MOVE_WALK, 2.0f, true);
            me->SetSpeed(MOVE_RUN, 2.0f, true);

            CheckTimer = 2000;
        }
        

        if (WhirlwindTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_WHIRLWIND);
            WhirlwindTimer =  urand(6000, 14000);
        }
        
        if (Whirlwind2Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_WHIRLWIND2);
            Whirlwind2Timer = urand(7000, 17000);
        }
        
        
        if (KnockdownTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_KNOCKDOWN);
            KnockdownTimer = urand(8000, 12000);
        }
 
        if (FrenzyTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FRENZY);
            FrenzyTimer = urand(20000, 25000);
        }

        if (CleaveTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_CLEAVE);
            CleaveTimer = 6000;
        }
        
        if (YellTimer.Expired(diff))
        {
            DoScriptText(RAND(YELL_RANDOM1, YELL_RANDOM2, YELL_RANDOM3, YELL_RANDOM4, YELL_RANDOM5), m_creature);
            YellTimer = urand(20000, 30000); //20 to 30 seconds
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

enum AVDrekTharOfficerSpells
{
    AV_DT_CHARGE    = 22911,
    AV_DT_CLEAVE    = 40504,
    AV_DT_DEMOSHOUT = 23511,
    AV_DT_WHIRLWIND = 13736,
    AV_DT_ENRAGE    = 8599
};

struct boss_drektharOfficerAI : public ScriptedAI
{
    boss_drektharOfficerAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }

    Timer_UnCheked chargeTimer;
    Timer_UnCheked cleaveTimer;
    Timer_UnCheked demoShoutTimer;
    Timer_UnCheked whirlwindTimer;
    Timer_UnCheked CheckTimer;
    WorldLocation wLoc;

    void Reset()
    {
        chargeTimer.Reset(urand(7500, 20000));
        cleaveTimer.Reset(urand(5000, 10000));
        demoShoutTimer.Reset(urand(2000, 4000));
        whirlwindTimer.Reset(urand(9000, 13000));
        CheckTimer.Reset(2000);
    }

    void EnterCombat(Unit *who)
    {
        // pull rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this, who] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->isAlive() && c->IsAIEnabled && c->AI())
                            c->AI()->AttackStart(who);
                  });
    }

    void JustRespawned()
    {
        Reset();
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        // evade rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->IsInCombat() && c->IsAIEnabled && c->AI())
                            c->AI()->EnterEvadeMode();
                  });
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        
        if (CheckTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 50.0f))
                EnterEvadeMode();

            me->SetSpeed(MOVE_WALK, 1.5f, true);
            me->SetSpeed(MOVE_RUN, 1.5f, true);

            CheckTimer = 2000;
        }
        
        if (chargeTimer.Expired(diff))
        {
            Unit * target = SelectUnit(SELECT_TARGET_RANDOM, 0, 25.0f, true, 0, 8.0f);

            if (target)
                AddSpellToCast(target, AV_DT_CHARGE);

            chargeTimer = urand(7500, 20000);
        }
        
        
        if (cleaveTimer.Expired(diff))
        {
            AddSpellToCast(AV_DT_CLEAVE, CAST_TANK);
            cleaveTimer = urand(5000, 10000);
        }
        
        
        if (demoShoutTimer.Expired(diff))
        {
            AddSpellToCast(AV_DT_DEMOSHOUT, CAST_NULL);
            demoShoutTimer = urand(14000, 25000);
        }
        
        
        if (whirlwindTimer.Expired(diff))
        {
            AddSpellToCast(AV_DT_WHIRLWIND, CAST_SELF);
            whirlwindTimer = urand(9000, 13000);
        }
        
           

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_drekthar(Creature *_Creature)
{
    return new boss_drektharAI (_Creature);
}

CreatureAI* GetAI_boss_drektharOfficer(Creature *_Creature)
{
    return new boss_drektharOfficerAI (_Creature);
}

void AddSC_boss_drekthar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_drekthar";
    newscript->GetAI = &GetAI_boss_drekthar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_drekthar_officer";
    newscript->GetAI = &GetAI_boss_drektharOfficer;
    newscript->RegisterSelf();
}
