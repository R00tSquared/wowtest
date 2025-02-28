/* ScriptData
SDName: boss_soccothrates_and_daliah
SD%Complete: 100
SDComment:
SDCategory: Tempest Keep, The Arcatraz
EndScriptData */

#include "precompiled.h"
#include "def_arcatraz.h"

enum Soccothrates
{
    // Intro yells
    SAY_SOCCOTHRATES_INTRO_1        = -1552049,
    SAY_DALLIAH_INTRO_2             = -1552050,
    SAY_SOCCOTHRATES_INTRO_3        = -1552051,
    SAY_DALLIAH_INTRO_4             = -1552052,
    SAY_SOCCOTHRATES_INTRO_5        = -1552053,
    SAY_DALLIAH_INTRO_6             = -1552054,
    SAY_SOCCOTHRATES_INTRO_7        = -1552055,

    SAY_AGGRO                       = -1552048,
    SAY_KILL                        = -1552047,
    SAY_DEATH                       = -1552046,
    SAY_CHARGE_1                    = -1552044,
    SAY_CHARGE_2                    = -1552045,

    SPELL_IMMOLATION                = 36051,
    SPELL_IMMOLATION_H              = 39007,
    SPELL_KNOCK_AWAY                = 36512,
    SPELL_FELFIRE_LINE_UP           = 35770,                // dummy spell - moves prespawned NPCs into a line
    SPELL_SUMMON_CHARGE_TARGET      = 36038,                // summons 21030 on target
    SPELL_CHARGE                    = 35754,                // script target on 21030; also dummy effect area effect target on 20978 - makes the target cast 35769
    SPELL_FELFIRE_SHOCK             = 35759,
    SPELL_FELFIRE_SHOCK_H           = 39006,

    NPC_CHARGE_TARGET               = 21030,
    NPC_WRATH_SCRYER_FELFIRE        = 20978,
    NPC_DALLIAH                     = 20885,
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_SOCCOTHRATES_INTRO_1,  NPC_SOCCOTHRATES,   3000},
    {SAY_DALLIAH_INTRO_2,       NPC_DALLIAH,        2000},
    {SAY_SOCCOTHRATES_INTRO_3,  NPC_SOCCOTHRATES,   4000},
    {SAY_DALLIAH_INTRO_4,       NPC_DALLIAH,        5000},
    {SAY_SOCCOTHRATES_INTRO_5,  NPC_SOCCOTHRATES,   3000},
    {SAY_DALLIAH_INTRO_6,       NPC_DALLIAH,        3000},
    {SAY_SOCCOTHRATES_INTRO_7,  NPC_SOCCOTHRATES,   0},
    {0, 0, 0},
};

static const float SoccotharesStartPos[4] = {122.1035, 192.7203, 22.44115, 5.235};
static const float DalliahStartPos[4] = {118.6038, 96.84682, 22.44115, 1.012};

#define TARGET_COUNT_SOCCOTHRATES 9

struct npc_soccothratesAI : public ScriptedAI, private DialogueHelper
{
    npc_soccothratesAI(Creature *c) : ScriptedAI(c), Summons(me), DialogueHelper(aIntroDialogue)
    {
        pInstance = c->GetInstanceData();
        HeroicMode = me->GetMap()->IsHeroic();
    }

    Timer Timer_FelfireShock;
    Timer Timer_KnockAway;
    Timer Timer_Charge;
    Timer Timer_FelfireLineUp;
    float x, y, z; // last charge target location
    uint8 lineUpCounter;
    uint8 fTargetCount;
    uint64 fTarget[TARGET_COUNT_SOCCOTHRATES];

    ScriptedInstance *pInstance;
    SummonList Summons;
    bool HeroicMode;

    void Reset()
    {
        Timer_FelfireShock.Reset(12000);
        Timer_KnockAway.Reset(23000);
        Timer_Charge.Reset(0);
        Timer_FelfireLineUp.Reset(0);
        DoCast(me, HeroicMode ? SPELL_IMMOLATION_H : SPELL_IMMOLATION);
        Summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_SOCCOTHRATES, NOT_STARTED);
        fTargetCount = 2;
        for (uint32 i = 0; i < TARGET_COUNT_SOCCOTHRATES; ++i)
            fTarget[i] = 0;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (pInstance && pInstance->GetData(TYPE_SOC_DAL_INTRO) != DONE)
        {
            if(pWho && pWho->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(pWho, 70.0f))
            {
                StartNextDialogueText(SAY_SOCCOTHRATES_INTRO_1);
                pInstance->SetData(TYPE_SOC_DAL_INTRO, DONE);
            }
        }
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        if (me->isAlive())
            me->SetHomePosition(SoccotharesStartPos[0], SoccotharesStartPos[1], SoccotharesStartPos[2], SoccotharesStartPos[3]);

        if (pInstance)
            pInstance->SetData(TYPE_SOCCOTHRATES, FAIL);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        // Adjust orientation
        if (PointId)
            me->SetFacingTo(SoccotharesStartPos[3]);
    }

    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(SAY_AGGRO, me);

        if(pInstance)
            pInstance->SetData(TYPE_SOCCOTHRATES, IN_PROGRESS);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(SAY_KILL, me);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        DoScriptText(SAY_DEATH, me);
        Summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_SOCCOTHRATES, DONE);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_CHARGE_TARGET)
        {
            summoned->GetPosition(x, y, z);
            lineUpCounter = 1;
        }
        Summons.Summon(summoned);
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_FELFIRE_LINE_UP)
        {
            uint64 hitGuid = target->GetGUID();
            for (uint32 i = 1; i < TARGET_COUNT_SOCCOTHRATES; ++i) // starting from the second in array. Just saving the previous logic
            {
                if (fTarget[i] == hitGuid)
                    lineUpCounter = i;
            }

            // need to get points between caster and target for reposition of felfire
            float sX = me->GetPositionX(), sY = me->GetPositionY(); // source coords
            float tX = x, tY = y; // target coords
            float felfireDistX = (tX - sX) / 7, felfireDistY = (tY - sY) / 7;
            float fX = sX + (felfireDistX * lineUpCounter), fY = sY + (felfireDistY * lineUpCounter);
            target->NearTeleportTo(fX, fY, me->GetPositionZ(), me->GetOrientation());
            target->CastSpell(target, 35769, false);
        }
        else if (spell->Id == SPELL_CHARGE && target->GetEntry() == NPC_CHARGE_TARGET)
            SetCombatMovement(true);
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        switch (uiEntry)
        {
            case NPC_SOCCOTHRATES:      return me;
            case NPC_DALLIAH:           return me->GetMap()->GetCreatureById(NPC_DALLIAH);
            default:
                return NULL;
        }

    }

    void JustDidDialogueStep(int32 iEntry)
    {
        // Move each of them to their places
        if (iEntry == SAY_SOCCOTHRATES_INTRO_7)
        {
            me->GetMotionMaster()->MovePoint(1, SoccotharesStartPos[0], SoccotharesStartPos[1], SoccotharesStartPos[2]);

            if (pInstance)
            {
                if (Creature* pDalliah = pInstance->GetCreatureById(NPC_DALLIAH))
                    pDalliah->GetMotionMaster()->MovePoint(1, DalliahStartPos[0], DalliahStartPos[1], DalliahStartPos[2]);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        DialogueUpdate(diff);
        if (!UpdateVictim())
            return;

        if(Timer_FelfireShock.Expired(diff))
        {
            AddSpellToCast(HeroicMode ? SPELL_FELFIRE_SHOCK_H : SPELL_FELFIRE_SHOCK, CAST_TANK);
            Timer_FelfireShock = urand(35000, 45000);
        }
        
        if(Timer_KnockAway.Expired(diff))
        {
            AddSpellToCast(SPELL_KNOCK_AWAY, CAST_TANK, false);
            Timer_KnockAway = 30000;
            Timer_FelfireLineUp = 2000;
        }
        
        if(Timer_FelfireLineUp.Expired(diff))
        {
            if(Unit* playerTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
            {
                me->SetSelection(playerTarget->GetGUID());
                DoCast(playerTarget, SPELL_SUMMON_CHARGE_TARGET, true);
            }

            if(Unit* pTarget = FindCreature(NPC_CHARGE_TARGET, 100, me))
            {
                float dist = me->GetDistance(pTarget);
                fTargetCount = (dist / 5.0f) + 1;
                if (fTargetCount < 2)
                    fTargetCount = 2;
                else if (fTargetCount > 9)
                    fTargetCount = 9;
            }
                
            for(int i = 0; i < fTargetCount;i++)
            {
                if (Creature* target = me->SummonCreature(NPC_WRATH_SCRYER_FELFIRE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 21000))
                    fTarget[i] = target->GetGUID();
            }

            DoScriptText(urand(0, 1) ? SAY_CHARGE_1 : SAY_CHARGE_2, me);

            AddSpellToCast(SPELL_FELFIRE_LINE_UP, CAST_NULL);

            Timer_Charge = 500;
            Timer_FelfireLineUp = 0;
        }
        
        if(Timer_Charge.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_KNOCK_AWAY);
            SetCombatMovement(false); // prevents interrupting charge
            AddSpellToCast(SPELL_CHARGE, CAST_NULL, true);
            Timer_Charge = 0;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_soccothratesAI(Creature *_Creature)
{
    return new npc_soccothratesAI(_Creature);
}

enum Daliah
{
    SAY_AGGRO_DALIAH                = -1552031,
    SAY_SOCCOTHRATES_TAUNT_1        = -1552040,
    SAY_SOCCOTHRATES_TAUNT_2        = -1552041,
    SAY_SOCCOTHRATES_TAUNT_3        = -1552042,
    SAY_HEAL_1                      = -1552032,
    SAY_HEAL_2                      = -1552033,
    SAY_KILL_1                      = -1552034,
    SAY_KILL_2                      = -1552035,
    SAY_WHIRLWIND_1                 = -1552036,
    SAY_WHIRLWIND_2                 = -1552037,
    SAY_DEATH_DALIAH                = -1552038,

    SPELL_GIFT_DOOMSAYER            = 36173,
    SPELL_GIFT_DOOMSAYER_H          = 39009,
    SPELL_HEAL                      = 36144,
    SPELL_HEAL_H                    = 39013,
    SPELL_WHIRLWIND                 = 36142,
    SPELL_SHADOW_WAVE               = 39016,                // heroic spell only
};

struct npc_daliahAI : public ScriptedAI
{
    npc_daliahAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
        HeroicMode = me->GetMap()->IsHeroic();
    }

    Timer Timer_GiftDoomsayer;
    Timer Timer_Heal;
    Timer Timer_Whirlwind;
    Timer Timer_ShadowWave;

    ScriptedInstance *pInstance;
    bool HeroicMode;
    bool HasTaunted;

    void Reset()
    {
        Timer_GiftDoomsayer.Reset(5000);
        Timer_Heal.Reset(0);
        Timer_Whirlwind.Reset(15000);
        Timer_ShadowWave.Reset(11000);
        HasTaunted = false;

        if(pInstance)
            pInstance->SetData(TYPE_DALLIAH, NOT_STARTED);
    }

    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(SAY_AGGRO_DALIAH, me);

        if(pInstance)
            pInstance->SetData(TYPE_DALLIAH, IN_PROGRESS);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(urand(0, 1) ? SAY_KILL_1 : SAY_KILL_2, me);
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        if (me->isAlive())
            me->GetMotionMaster()->MovePoint(1, DalliahStartPos[0], DalliahStartPos[1], DalliahStartPos[2]);

        if (pInstance)
            pInstance->SetData(TYPE_DALLIAH, FAIL);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        DoScriptText(SAY_DEATH_DALIAH, me);
        if(pInstance)
            pInstance->SetData(TYPE_DALLIAH, DONE);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        // Adjust orientation
        if (PointId)
            me->SetFacingTo(DalliahStartPos[3]);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(Timer_GiftDoomsayer.Expired(diff))
        {
            AddSpellToCast(HeroicMode ? SPELL_GIFT_DOOMSAYER_H : SPELL_GIFT_DOOMSAYER, CAST_TANK);
            Timer_GiftDoomsayer = 17000;
        }

        if(Timer_Whirlwind.Expired(diff))
        {
            AddSpellToCast(SPELL_WHIRLWIND, CAST_NULL);
            DoScriptText(urand(0, 1) ? SAY_WHIRLWIND_1 : SAY_WHIRLWIND_2, me);
            Timer_Whirlwind = 25000;
            Timer_Heal = 8000;
        }

        if(Timer_Heal.Expired(diff))
        {
            AddSpellToCast(HeroicMode ? SPELL_HEAL_H : SPELL_HEAL, CAST_SELF);
            DoScriptText(urand(0, 1) ? SAY_HEAL_1 : SAY_HEAL_2, me);
            Timer_Heal = 0;
        }

        if(HeroicMode)
        {
            if(Timer_ShadowWave.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_FARTHEST, 0, GetSpellMaxRange(SPELL_SHADOW_WAVE), true))
                    AddSpellToCast(target, SPELL_SHADOW_WAVE);
                Timer_ShadowWave = 15000;
            }
        }

        if(!HasTaunted)
        {
            if(me->GetHealthPercent() < 25)
            {
                if(pInstance && pInstance->GetData(TYPE_SOCCOTHRATES) != DONE)
                {
                    if(Creature* pSoccothrates = pInstance->GetCreatureById(NPC_SOCCOTHRATES))
                    {
                        switch (urand(0, 2))
                        {
                            case 0: DoScriptText(SAY_SOCCOTHRATES_TAUNT_1, pSoccothrates); break;
                            case 1: DoScriptText(SAY_SOCCOTHRATES_TAUNT_2, pSoccothrates); break;
                            case 2: DoScriptText(SAY_SOCCOTHRATES_TAUNT_3, pSoccothrates); break;
                        }
                    }
                }
                HasTaunted = true;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_daliahAI(Creature *_Creature)
{
    return new npc_daliahAI(_Creature);
}

void AddSC_boss_soccothrates_and_daliah()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_soccothrates";
    newscript->GetAI = &GetAI_npc_soccothratesAI;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_daliah";
    newscript->GetAI = &GetAI_npc_daliahAI;
    newscript->RegisterSelf();
}