// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
// script is owned by DeathSide, Trentone
#define SPELL_HAMSTRING     38995
#define SPELL_REND            29574
#define SPELL_CHARGE        35382
#define SPELL_SHIELD_SLAM    46762


#define SOUND_AGGRO_ORC_MALE            2694
#define SOUND_AGGRO_ORC_FEMALE            2706    
#define SOUND_AGGRO_HUMAN_MALE            2670
#define SOUND_AGGRO_HUMAN_FEMALE        2682
#define SOUND_HELP_ORC_MALE                2692        
#define SOUND_HELP_ORC_FEMALE            2704    
#define SOUND_HELP_HUMAN_MALE            2668
#define SOUND_HELP_HUMAN_FEMALE            2680
#define SOUND_ATTACK_ORC_MALE            2696
#define SOUND_ATTACK_ORC_FEMALE            2708    
#define SOUND_ATTACK_HUMAN_MALE            2672
#define SOUND_ATTACK_HUMAN_FEMALE        2684
#define MODEL_ORC_MALE                    15821
#define MODEL_ORC_FEMALE                14362    
#define MODEL_HUMAN_MALE                18446
#define MODEL_HUMAN_FEMALE                18447

struct deathside_guard_orgrimmar_and_stormwindAI : public ScriptedAI
{
    deathside_guard_orgrimmar_and_stormwindAI(Creature *c) : ScriptedAI(c) {}

    uint32 SPELL_SHIELD_SLAM_TIMER;
    uint32 SPELL_HAMSTRING_TIMER;
    uint32 SPELL_REND_TIMER;
    uint32 SPELL_CHARGE_TIMER;
    bool HasYelledForHelp;

    void Reset()
    {
        SPELL_CHARGE_TIMER = 100;
        SPELL_HAMSTRING_TIMER = 100;
        SPELL_REND_TIMER = 100;
        SPELL_SHIELD_SLAM_TIMER = 100;
        HasYelledForHelp = false;
    }

    void EnterCombat(Unit * /*who*/)
    {
        if (me->getFaction() == 85)
        {
            if(me->GetDisplayId() == MODEL_ORC_MALE)
                DoPlaySoundToSet(me, SOUND_AGGRO_ORC_MALE);
            else
                DoPlaySoundToSet(me, SOUND_AGGRO_ORC_FEMALE);
        }
        else
        {
            if (me->GetDisplayId() == MODEL_HUMAN_MALE)
                DoPlaySoundToSet(me, SOUND_AGGRO_HUMAN_MALE);
            else
                DoPlaySoundToSet(me, SOUND_AGGRO_HUMAN_FEMALE);
        }
    }

    void JustDied(Unit* Killer)
    {
        if (Player* pKiller = Killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        me->SendZoneUnderAttackMessage(pKiller);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!HasYelledForHelp && me->GetHealth() <= me->GetMaxHealth()*0.3)
        {
            if (rand()%3)
            {
                if (me->getFaction() == 85)
                {
                    if(me->GetDisplayId() == MODEL_ORC_MALE)
                        DoPlaySoundToSet(me, SOUND_HELP_ORC_MALE);
                    else
                        DoPlaySoundToSet(me, SOUND_HELP_ORC_FEMALE);
                }
                else
                {
                    if (me->GetDisplayId() == MODEL_HUMAN_MALE)
                        DoPlaySoundToSet(me, SOUND_HELP_HUMAN_MALE);
                    else
                        DoPlaySoundToSet(me, SOUND_HELP_HUMAN_FEMALE);
                }
            }
            HasYelledForHelp = true;
        }

        if (SPELL_HAMSTRING_TIMER <= diff)
        {
            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                DoCast(me->GetVictim(), SPELL_HAMSTRING);
                SPELL_HAMSTRING_TIMER = urand(6000,8000);
            }
        } else SPELL_HAMSTRING_TIMER -= diff;

        if (SPELL_REND_TIMER <= diff)
        {
            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                DoCast(me->GetVictim(), SPELL_REND);
                SPELL_REND_TIMER = 15000;
            }
        } else SPELL_REND_TIMER -= diff;

        if (SPELL_CHARGE_TIMER <= diff)
        {
            Unit *ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 1, 40, false);    // Going for casting target (10 tries)
            if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
            {
                ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 2, 40, false);
                if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                {
                        ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 3, 40, false);
                    if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                    {
                        ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 4, 40, false);
                        if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                        {
                            ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 5, 40, false);
                            if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                            {
                                ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 6, 40, false);
                                if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                                {
                                    ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 7, 40, false);
                                    if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                                    {
                                        ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 8, 40, false);
                                        if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                                        {
                                            ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 9, 40, false);
                                            if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))
                                            {
                                                ChargeTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 10, 40, false);    
                                                if (ChargeTarget && !ChargeTarget->HasUnitState(UNIT_STAT_CASTING))    
                                                    ChargeTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, 40, false); // if last try target is not casting then target anyone in 40 yd distance (except tank)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!ChargeTarget)    // if there's no target, then the target is tank
                ChargeTarget = me->GetVictim();
            if (ChargeTarget && me->GetDistance(ChargeTarget) <= 40 )    // start casting only if we're 40yd distance or lower to our target
            {
                if (me->getFaction() == 85)
                {
                    if(me->GetDisplayId() == MODEL_ORC_MALE)
                        DoPlaySoundToSet(me, SOUND_ATTACK_ORC_MALE);
                    else
                        DoPlaySoundToSet(me, SOUND_ATTACK_ORC_FEMALE);
                }
                else
                {
                    if (me->GetDisplayId() == MODEL_HUMAN_MALE)
                        DoPlaySoundToSet(me, SOUND_ATTACK_HUMAN_MALE);
                    else
                        DoPlaySoundToSet(me, SOUND_ATTACK_HUMAN_FEMALE);
                }
                DoResetThreat();    // reset threat
                me->AddThreat(ChargeTarget, 5000);    // add 5000 threat to our new target
                DoCast(ChargeTarget, SPELL_CHARGE);    // attack it
                SPELL_CHARGE_TIMER = urand(13000,19000); // new spell_charge timer
            }
        } else SPELL_CHARGE_TIMER -= diff;

        if (SPELL_SHIELD_SLAM_TIMER <= diff)    
        {
            Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 5, false);
            if (random)
            {
                DoCast(random, SPELL_SHIELD_SLAM);
                SPELL_SHIELD_SLAM_TIMER = urand(6000,14000);
            }
        } else SPELL_SHIELD_SLAM_TIMER -= diff;

        DoMeleeAttackIfReady();
    }
    void ReceiveEmote(Player* pPlayer, uint32 text_emote)
    {
        if ((pPlayer->GetTeam() == HORDE && me->getFaction() == 85) || (pPlayer->GetTeam() == ALLIANCE && me->getFaction() == 11))
        {
            switch(text_emote)
            {
                case TEXTEMOTE_CHICKEN: me->HandleEmoteCommand(EMOTE_ONESHOT_RUDE);    break;
                case TEXTEMOTE_DANCE:
                case TEXTEMOTE_FLEX:    me->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);    break;
                case TEXTEMOTE_LOVE:
                case TEXTEMOTE_KISS:
                {
                    if ((pPlayer->GetGender() == GENDER_MALE && (me->GetDisplayId() == MODEL_ORC_MALE || me->GetDisplayId() == MODEL_HUMAN_MALE)) || (pPlayer->GetGender() == GENDER_FEMALE && (me->GetDisplayId() == MODEL_ORC_FEMALE || me->GetDisplayId() == MODEL_HUMAN_FEMALE)))
                        me->HandleEmoteCommand(EMOTE_ONESHOT_RUDE);
                    else
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
                    break;
                }
                case TEXTEMOTE_RUDE:
                case TEXTEMOTE_POINT:
                case TEXTEMOTE_LAUGH:    me->HandleEmoteCommand(EMOTE_ONESHOT_POINT);    break;
                case TEXTEMOTE_ROAR:    me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);    break;
                case TEXTEMOTE_SALUTE:    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE); break;
                case TEXTEMOTE_TALK:    me->HandleEmoteCommand(EMOTE_ONESHOT_TALK); break;
                case TEXTEMOTE_WAVE:    me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);   break;
            }
        }
    }

};
CreatureAI* GetAI_deathside_guard_orgrimmar_and_stormwindAI(Creature* pCreature)
{
return new deathside_guard_orgrimmar_and_stormwindAI (pCreature);
}

void AddSC_deathside_guard_orgrimmar_and_stormwind()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "deathside_guard_orgrimmar_and_stormwind";
    newscript->GetAI = &GetAI_deathside_guard_orgrimmar_and_stormwindAI;
    newscript->RegisterSelf();
}
