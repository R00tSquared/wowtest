#include "precompiled.h"
#include "def_underbog.h"

enum
{
    SAY_AGGRO_1 = -1100101,
    SAY_AGGRO_2 = -1100102,
    SAY_AGGRO_3 = -1100103,
    SAY_KILL_1 = -1100104,
    SAY_KILL_2 = -1100105,
    SAY_DEATH = -1100106,
    SAY_BEAST = -1100107,

    SPELL_AIMED_SHOT = 31623,
    SPELL_BEAR_COMMAND = 34662,
    SPELL_DETERRENCE = 31567,
    SPELL_FREEZING_TRAP_EFFECT = 31932,
    SPELL_HUNTERS_MARK = 31615,
    SPELL_THROW_FREEZING_TRAP = 31946,

    // Handled by behavioral AI:
    SPELL_KNOCK_AWAY = 18813,
    SPELL_MULTISHOT = 34974,
    SPELL_SHOOT = 22907,

    SPELL_NOTIFY_OF_DEATH = 31547, // ??

    SPELL_FERAL_CHARGE = 39435,
    SPELL_ECHOING_ROAR = 31429,
    SPELL_FRENZY = 34971,
    SPELL_MAUL = 34298,
    NPC_CLAW = 17827,
    NPC_WINDCALLER_CLAW = 17894,
    NPC_SWAMPLORD_MUSELEK = 17826
};

struct boss_swamplord_muselekAI : public ScriptedAI
{
    boss_swamplord_muselekAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;

    bool HeroicMode;
    ObjectGuid aimedShotVictimance;

    uint32 FreezingTrapTimer;
    uint32 HuntersMarkTimer;
    uint32 AimedShotTimer;
    uint32 KnockAwayTimer;
    uint32 MultiShotTimer;
    uint32 ShootTimer;

    uint32 BearCommandTimer;
    uint32 DeterrenceTimer;
    bool DoingStaff;

    void EvadeHome()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void EnterEvadeMode() override;

    void Reset()
    {
        aimedShotVictimance = ObjectGuid();

        FreezingTrapTimer = 25000;
        HuntersMarkTimer = 0;
        AimedShotTimer = 0;

        BearCommandTimer = 7000;
        DeterrenceTimer = 30000;
        KnockAwayTimer = 30000;
        MultiShotTimer = 21000;
        ShootTimer = 100;
        DoingStaff = false;
        if(pInstance)
            pInstance->SetData(TYPE_SWAMPLORD, NOT_STARTED);
    }

    void EnterCombat(Unit* pWho)
    {
        ScriptedAI::AttackStartNoMove(pWho, CHECK_TYPE_SHOOTER);
        if (pInstance)
        {
            if (Creature* pClaw = pInstance->GetCreatureById(NPC_CLAW))
                if (pClaw->AI())
                    pClaw->AI()->AttackStart(pWho);
        }

        switch (urand(0, 2))
        {
            case 0:
                DoScriptText(SAY_AGGRO_1, me);
                break;
            case 1:
                DoScriptText(SAY_AGGRO_2, me);
                break;
            case 2:
                DoScriptText(SAY_AGGRO_3, me);
                break;
        }
        if(pInstance)
            pInstance->SetData(TYPE_SWAMPLORD, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), me);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        DoScriptText(SAY_DEATH, me);
        if(pInstance)
            pInstance->SetData(TYPE_SWAMPLORD, DONE);
    }

    void JustReachedHome()
    {
        if (pInstance)
        {
            if (Creature* pClaw = pInstance->GetCreatureById(NPC_CLAW))
            {
                pClaw->UpdateEntry(NPC_CLAW, ALLIANCE);
                pClaw->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pClaw->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            }
        }
    }

    void SpellHitTarget(Unit* target, const  SpellEntry * spell)
    {
        if(spell->Id == SPELL_AIMED_SHOT)
        {
            me->SetRooted(false);
            DoingStaff = false;
        }
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32& /*ui*/)
    {
        if (!DeterrenceTimer)
        {
            DoCast(me, SPELL_DETERRENCE);
            DeterrenceTimer = 30000;
        }
    }

    Unit* PickAimedShotVictim(bool forHuntersMark)
    {
        if (forHuntersMark)
            aimedShotVictimance = me->GetVictim()->GetObjectGuid();

        if (aimedShotVictimance)
            if (Unit* pTarget = Unit::GetUnit((*me), aimedShotVictimance))
                if (pTarget->isAlive())
                    return pTarget;

        std::list<Unit*> targets;
        std::list<HostileReference*>::iterator i = me->getThreatManager().getThreatList().begin();
        for (; i != me->getThreatManager().getThreatList().end(); ++i)
        {
            Unit* target = Unit::GetUnit(*me, (*i)->getUnitGuid());
            if (target->HasAura(SPELL_FREEZING_TRAP_EFFECT))
                targets.push_back(target);
        }

        if (targets.empty())
            return NULL;

        std::list<Unit*>::iterator j = targets.begin();
        if (targets.size() > 1)
            advance ( j , rand() % (targets.size() - 1) );

        aimedShotVictimance = (*j)->GetObjectGuid();
        return (*j);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id == 100)
        {
            AimedShotTimer = 1000;
            me->SetRooted(true);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        // Freezing Trap
        if (FreezingTrapTimer <= uiDiff)
        {
            DoCast(me, SPELL_THROW_FREEZING_TRAP);
            HuntersMarkTimer = 1600;
            // AimedShotTimer = 3200;
            FreezingTrapTimer = 40000;
            Position dest;
            me->GetValidPointInAngle(dest, urand(8.0f, 10.0f), frand(0.0f, 2*M_PI), true);
            me->GetMotionMaster()->MovePoint(100, dest.x, dest.y, dest.z);
            DoingStaff = true;
        }
        else
            FreezingTrapTimer -= uiDiff;

        if (HuntersMarkTimer)
        {
            if (HuntersMarkTimer <= uiDiff)
            {
                if (Unit* pTarget = PickAimedShotVictim(true))
                {
                    DoCast(pTarget, SPELL_HUNTERS_MARK);
                    HuntersMarkTimer = 0;
                }
                else
                    HuntersMarkTimer = 0;
            }
            else
                HuntersMarkTimer -= uiDiff;
        }

        if (AimedShotTimer)
        {
            if (AimedShotTimer <= uiDiff)
            {
                if (Unit* pTarget = PickAimedShotVictim(false))
                {
                    DoCast(pTarget, SPELL_AIMED_SHOT);
                    AimedShotTimer = 0;
                }
                else
                    AimedShotTimer = 0;
            }
            else
                AimedShotTimer -= uiDiff;
        }

        if(DoingStaff)
            return;

        if(KnockAwayTimer <= uiDiff)
        {
            DoCast(me->GetVictim(), SPELL_KNOCK_AWAY);
            KnockAwayTimer = urand(30000, 40000);
        }
        else
            KnockAwayTimer -= uiDiff;

        if(MultiShotTimer <= uiDiff)
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if (target && !me->IsWithinDist(target, 5.0f))
            {
                DoCast(target, SPELL_MULTISHOT);
                MultiShotTimer = urand(20000, 30000);
            }
            else
                MultiShotTimer = 3000;
        }
        else
            MultiShotTimer -= uiDiff;

        if(ShootTimer <= uiDiff)
        {
            if(me->GetVictim() && !me->IsWithinDist(me->GetVictim(), 5.0f))
                DoCast(me->GetVictim(), SPELL_SHOOT);
            ShootTimer = 2100;
        }
        else
            ShootTimer -= uiDiff;

        // Bear Command
        if (BearCommandTimer <= uiDiff)
        {
            if (pInstance)
            {
                if (Creature* pClaw = pInstance->GetCreatureById(NPC_CLAW))
                {
                    if (pClaw->GetEntry() == NPC_CLAW && pClaw->isAlive())
                    {
                        DoScriptText(SAY_BEAST, me);
                        pClaw->CastSpell(pClaw, SPELL_FRENZY, true);
                    }
                }
                BearCommandTimer = 30000;
            }
        }
        else
            BearCommandTimer -= uiDiff;

        // Update Deterrence timer
        if (DeterrenceTimer <= uiDiff)
            DeterrenceTimer = 0;
        else
            DeterrenceTimer -= uiDiff;

        CheckShooterNoMovementInRange(uiDiff, 30.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_swamplord_muselek(Creature* pCreature)
{
    return new boss_swamplord_muselekAI(pCreature);
}

struct boss_clawAI : public ScriptedAI
{
    boss_clawAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;
    bool HeroicMode;
    uint32 FeralChargeTimer;
    uint32 MaulTimer;
    uint32 EchoingRoarTimer;

    void EnterCombat(Unit* pWho)
    {
        if (pInstance)
        {
            if (auto muselek = pInstance->GetCreatureById(NPC_SWAMPLORD_MUSELEK))
                if (muselek->AI())
                    muselek->AI()->AttackStart(pWho);
        }
    }

    void EvadeHome() { ScriptedAI::EnterEvadeMode(); }

    void EnterEvadeMode() override
    {
        if (pInstance)
            if (auto muselek = pInstance->GetCreatureById(NPC_SWAMPLORD_MUSELEK))
                if (auto ai = dynamic_cast<boss_swamplord_muselekAI*>(muselek->AI()))
                    ai->EvadeHome();
        ScriptedAI::EnterEvadeMode();
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (me->GetEntry() == NPC_WINDCALLER_CLAW)
            return;

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void BecomeClaw()
    {
        me->UpdateEntry(NPC_WINDCALLER_CLAW, ALLIANCE); // For some reason it should be ALLIANCE as team model id
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
    }

    void Reset()
    {
        MaulTimer = 10000;
        EchoingRoarTimer = 6000;
        FeralChargeTimer = 1000;
    }

    void JustReachedHome()
    {
        // Sit if we're in druid form
        if (me->GetEntry() == NPC_WINDCALLER_CLAW)
            me->SetStandState(UNIT_STAND_STATE_SIT);
        else
        {
            if (pInstance)
                if (Creature* muselek = pInstance->GetCreatureById(NPC_SWAMPLORD_MUSELEK))
                {
                    if (!muselek->isAlive())
                    {
                        BecomeClaw();
                        me->SetStandState(UNIT_STAND_STATE_SIT);
                    }
                }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        // If below 20% hp, turn into windcaller claw
        if (me->GetHealthPercent() <= 20)
        {
            // Become Windcaller Claw
            BecomeClaw();
            // Exit combat (call base class, our evade has special logic in it)
            ScriptedAI::EnterEvadeMode();
            return;
        }

        if(me->GetVictim()->HasAura(SPELL_FREEZING_TRAP_EFFECT))
            return;

        if (me->HasAura(SPELL_FRENZY))
        {
            if (FeralChargeTimer <= uiDiff)
            {
                if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, GetSpellMaxRange(SPELL_FRENZY)))
                    DoCast(pTarget, SPELL_FERAL_CHARGE);
                FeralChargeTimer = urand(2000, 4000);
            }
            else
                FeralChargeTimer -= uiDiff;
        }

        // Echoing Roar
        if (EchoingRoarTimer <= uiDiff)
        {
            DoCast(me, SPELL_ECHOING_ROAR);
            EchoingRoarTimer = urand(10000, 20000);
        }
        else
            EchoingRoarTimer -= uiDiff;

        // Maul
        if (MaulTimer <= uiDiff)
        {
            DoCast(me->GetVictim(), SPELL_MAUL);
            MaulTimer = urand(10000, 20000);
        }
        else
            MaulTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

void boss_swamplord_muselekAI::EnterEvadeMode()
{
    if (pInstance)
        if (auto claw = pInstance->GetCreatureById(NPC_CLAW))
            if (auto ai = dynamic_cast<boss_clawAI*>(claw->AI()))
                ai->EvadeHome();
    ScriptedAI::EnterEvadeMode();
}

CreatureAI* GetAI_boss_claw(Creature* pCreature)
{
    return new boss_clawAI(pCreature);
}

void AddSC_boss_swamplord_muselek()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_swamplord_muselek";
    pNewScript->GetAI = &GetAI_boss_swamplord_muselek;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_claw";
    pNewScript->GetAI = &GetAI_boss_claw;
    pNewScript->RegisterSelf();
}