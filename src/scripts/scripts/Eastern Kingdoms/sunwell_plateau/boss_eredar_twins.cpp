// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2009 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Eredar_Twins
SD%Complete: 95
SDComment: final debugging
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"
#include "GameEvent.h"

enum Quotes
{
    //Alytesh
    YELL_CANFLAGRATION          =   -1580044,
    YELL_SISTER_SACROLASH_DEAD  =   -1580045,
    YELL_ALY_KILL_1             =   -1580046,
    YELL_ALY_KILL_2             =   -1580047,
    YELL_ALY_DEAD               =   -1580048,
    YELL_BERSERK                =   -1580049,

    //Sacrolash
    YELL_SHADOW_NOVA            =   -1580050,
    YELL_SISTER_ALYTHESS_DEAD   =   -1580051,
    YELL_SAC_KILL_1             =   -1580052,
    YELL_SAC_KILL_2             =   -1580053,
    SAY_SAC_DEAD                =   -1580054,
    YELL_ENRAGE                 =   -1580055,

    //Intro
    YELL_INTRO_SAC_1            =   -1580056,
    YELL_INTRO_ALY_2            =   -1580057,
    YELL_INTRO_SAC_3            =   -1580058,
    YELL_INTRO_ALY_4            =   -1580059,
    YELL_INTRO_SAC_5            =   -1580060,
    YELL_INTRO_ALY_6            =   -1580061,
    YELL_INTRO_SAC_7            =   -1580062,
    YELL_INTRO_ALY_8            =   -1580063,

    //Emote
    EMOTE_SHADOW_NOVA           =   -1580064,
    EMOTE_CONFLAGRATION         =   -1580065
};

enum Spells
{
    //Lady Sacrolash spells
    SPELL_DUAL_WIELD        =   29651,
    SPELL_SHADOWFORM        =   45455,
    SPELL_DARK_TOUCHED      =   45347,
    SPELL_SHADOW_BLADES     =   45248, //10 secs
    SPELL_DARK_STRIKE       =   45271,
    SPELL_SHADOW_NOVA       =   45329, //30-35 secs
    SPELL_CONFOUNDING_BLOW  =   45256, //25 secs

    //Shadow Image spells
    SPELL_SHADOW_FURY       =   45270,
    SPELL_IMAGE_VISUAL      =   45263,

    //Misc spells
    SPELL_ENRAGE            =   46587,
    SPELL_EMPOWER           =   45366,
    SPELL_DARK_FLAME        =   45345,

    //Grand Warlock Alythess spells
    SPELL_FIREFORM          =   45457,
    SPELL_PYROGENICS        =   45230, //15secs
    SPELL_FLAME_TOUCHED     =   45348,
    SPELL_CONFLAGRATION     =   45342, //30-35 secs
    SPELL_BLAZE             =   45235, //on main target every 3 secs
    SPELL_FLAME_SEAR        =   46771
};

enum Creatures
{
    GRAND_WARLOCK_ALYTHESS  =   25166,
    MOB_SHADOW_IMAGE        =   25214,
    LADY_SACROLASH          =   25165
};
/*
enum NPCPath
{
    PATH_TRASH_WAVE1        =  ??,
    PATH_TRASH_WAVE2        =  ??,
    PATH_TRASH_WAVE3        =  ??,
    PATH_TRASH_WAVE4        =  ??
};*/

struct boss_sacrolashAI : public ScriptedAI
{
    boss_sacrolashAI(Creature *c) : ScriptedAI(c), Summons(me)
    {
        pInstance = c->GetInstanceData();
    }

    InstanceData *pInstance;
    SummonList Summons;

    Timer_UnCheked ShadowbladesTimer;
    Timer_UnCheked SpecialTimer;
    Timer_UnCheked ConfoundingblowTimer;
    Timer_UnCheked ShadowimageTimer;
    Timer_UnCheked EnrageTimer;

    void Reset()
    {
        ClearCastQueue();
        ShadowbladesTimer.Reset(10000);
        SpecialTimer.Reset(30000);
        ConfoundingblowTimer.Reset(25000);
        ShadowimageTimer.Reset(14000);
        EnrageTimer.Reset(360000);
        DoCast(me, SPELL_SHADOWFORM);
        DoCast(me, SPELL_DUAL_WIELD);

        if (pInstance->GetData(DATA_EREDAR_TWINS_INTRO) == DONE)
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

        pInstance->SetData(DATA_EREDAR_TWINS_EVENT, NOT_STARTED);
        pInstance->SetData(DATA_SACROLASH, NOT_STARTED);
    }

    void EnterEvadeMode()
    {
        if (pInstance->GetData(DATA_ALYTHESS) == DONE)
        {
            if(Unit* Alythess = me->GetUnit(pInstance->GetData64(DATA_ALYTHESS)))
                Alythess->ToCreature()->Respawn();
            pInstance->SetData(DATA_ALYTHESS, NOT_STARTED);
        }

        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit *who)
    {
        if(isGameEventActive(50)) // first gates aren't destroyed yet
        {
            int32 BP = 10000;
            me->CastCustomSpell(me, 45078, &BP, 0, 0, true); // 100 times stronger than normal on norm realm, 10 times stronger than normal on x100 realm. - Death anyway
        }
        DoZoneInCombat();
        pInstance->SetData(DATA_EREDAR_TWINS_EVENT, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (pInstance->GetData(DATA_EREDAR_TWINS_INTRO) == DONE)
            ScriptedAI::MoveInLineOfSight(who);
    }

    void DamageMade(Unit* target, uint32 &damage, bool direct_damage, uint8 school_mask)
    {
        if(target->GetTypeId() == TYPEID_PLAYER && damage)
        {
            if(school_mask == SPELL_SCHOOL_MASK_SHADOW)
            {
                SpellEntry* DarkTouched = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DARK_TOUCHED);
                if(target->HasAura(SPELL_FLAME_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }
                if(!target->HasAura(SPELL_DARK_FLAME) && !target->ToPlayer()->GetCooldownMgr().HasSpellIdCooldown(DarkTouched))
                {
                    target->CastSpell(target, SPELL_DARK_TOUCHED, true);
                    target->ToPlayer()->GetCooldownMgr().AddSpellIdCooldown(DarkTouched, 1000);
                }
            }
            if(school_mask == SPELL_SCHOOL_MASK_FIRE)
            {
                SpellEntry* FlameTouched = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(SPELL_FLAME_TOUCHED);
                if(target->HasAura(SPELL_DARK_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_DARK_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }
                if(!target->HasAura(SPELL_DARK_FLAME) && !target->ToPlayer()->GetCooldownMgr().HasSpellIdCooldown(FlameTouched))
                {
                    target->CastSpell(target, SPELL_FLAME_TOUCHED, true, NULL, NULL, me->GetGUID());
                    target->ToPlayer()->GetCooldownMgr().AddSpellIdCooldown(FlameTouched, 2800);
                }
            }
        }
    }

    void KilledUnit(Unit *victim)
    {
        if (roll_chance_i(20))
            DoScriptText(RAND(YELL_SAC_KILL_1, YELL_SAC_KILL_2), me);
    }

    void RemoveEncounterDebuffs()
    {
        Map *map = me->GetMap();
        if (!map->IsDungeon())
            return;

        Map::PlayerList const &PlayerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        for(i = PlayerList.begin(); i != PlayerList.end(); ++i)
            if (Player* i_pl = i->getSource())
            {
                i_pl->RemoveAurasDueToSpell(SPELL_DARK_TOUCHED);
                i_pl->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
            }
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance->GetData(DATA_ALYTHESS) == DONE)
        {
            DoScriptText(SAY_SAC_DEAD, me);
            pInstance->SetData(DATA_EREDAR_TWINS_EVENT, DONE);
            RemoveEncounterDebuffs();
        }
        else
            me->SetLootRecipient(NULL);

        Summons.DespawnAll();
        pInstance->SetData(DATA_SACROLASH, DONE);
    }

    void JustSummoned(Creature* summoned)
    {
        Summons.Summon(summoned);
    }

    void DoAction(const int32 action)
    {
        if (action == SISTER_DEATH)
            AddSpellToCastWithScriptText(SPELL_EMPOWER, CAST_SELF, YELL_SISTER_ALYTHESS_DEAD);
    }

    // searches for one of 5 top threat targets from sisters' threat list, but not her main target
    Unit* GetNovaTarget()
    {
        if(Creature* Alythess = me->GetCreature(pInstance->GetData64(DATA_ALYTHESS)))
        {
            Unit* target = Alythess->AI()->SelectUnit(SELECT_TARGET_TOPAGGRO, urand(1,5), 300.0f, true, Alythess->getVictimGUID());
            if(target && target->isAlive())
                return target;
        }
        return NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpecialTimer.Expired(diff))
        {
            if (pInstance->GetData(DATA_ALYTHESS) == DONE)
            {
                AddSpellToCastWithScriptText(SPELL_CONFLAGRATION, CAST_RANDOM_WITHOUT_TANK, EMOTE_CONFLAGRATION, false, true);
                SpecialTimer = urand(14000, 16000);
            }
            else
            {
                if(Unit* target = GetNovaTarget())
                    AddSpellToCastWithScriptText(target, SPELL_SHADOW_NOVA, EMOTE_SHADOW_NOVA, false, true);
                DoScriptText(YELL_SHADOW_NOVA, me);
                SpecialTimer = urand(30000,35000);
            }
        }


        
        if (ConfoundingblowTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_CONFOUNDING_BLOW, CAST_TANK);
            ConfoundingblowTimer = urand(20000, 25000);
        }
        

        
        if (ShadowimageTimer.Expired(diff))
        {
            for (int i = 0; i < 3; i++)
                DoSpawnCreature(MOB_SHADOW_IMAGE, 0, 0 , 0, frand(0, 2*M_PI), TEMPSUMMON_TIMED_DESPAWN, 15000);
            ShadowimageTimer = 20000;
        }
        

        if (ShadowbladesTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHADOW_BLADES, CAST_SELF);
            ShadowbladesTimer = 10000;
        }
        

        
        if (EnrageTimer.Expired(diff))
        {
            AddSpellToCastWithScriptText(SPELL_ENRAGE, CAST_SELF, YELL_ENRAGE);
            EnrageTimer = 360000;
        }
        

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_boss_sacrolash(Creature *_Creature)
{
    return new boss_sacrolashAI (_Creature);
};

struct boss_alythessAI : public Scripted_NoMovementAI
{
    boss_alythessAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
        IntroStepCounter = 10;
    }

    InstanceData *pInstance;

    bool IntroDone, TrashWaveDone;

    uint32 IntroStepCounter;
    Timer_UnCheked IntroYellTimer;

    Timer_UnCheked SpecialTimer;
    Timer_UnCheked PyrogenicsTimer;
    Timer_UnCheked FlamesearTimer;
    Timer_UnCheked EnrageTimer;

    void Reset()
    {
        ClearCastQueue();
        SpecialTimer.Reset(urand(15000, 19000));
        PyrogenicsTimer.Reset(15000);
        EnrageTimer.Reset(360000);
        FlamesearTimer.Reset(urand(10000, 15000));
        IntroYellTimer.Reset(500);
        IntroStepCounter = 10;

        IntroDone = false;
        TrashWaveDone = false;

        DoCast(me, SPELL_FIREFORM);

        if (pInstance->GetData(DATA_EREDAR_TWINS_INTRO) == DONE)
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

        pInstance->SetData(DATA_EREDAR_TWINS_EVENT, NOT_STARTED);
        pInstance->SetData(DATA_ALYTHESS, NOT_STARTED);

        SetAutocast(SPELL_BLAZE, 2700, true);
    }

    void EnterEvadeMode()
    {
        if (pInstance->GetData(DATA_SACROLASH) == DONE)
        {
            if(Unit* Sacrolash = me->GetUnit(pInstance->GetData64(DATA_SACROLASH)))
               Sacrolash->ToCreature()->Respawn();
            pInstance->SetData(DATA_SACROLASH, NOT_STARTED);
        }

        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit *who)
    {
        if(isGameEventActive(50)) // first gates aren't destroyed yet
        {
            int32 BP = 10000;
            me->CastCustomSpell(me, 45078, &BP, 0, 0, true); // 100 times stronger than normal on norm realm, 10 times stronger than normal on x100 realm. - Death anyway
        }
        DoZoneInCombat();
        pInstance->SetData(DATA_EREDAR_TWINS_EVENT, IN_PROGRESS);
        me->AddThreat(who, 0.1f);
    }

    void SetData(uint32 a, uint32 b)
    {
        if (a == 1 && b == 1 && pInstance->GetData(DATA_EREDAR_TWINS_INTRO) == NOT_STARTED && pInstance->GetData(DATA_FELMYST_EVENT) == DONE)
        {
            IntroStepCounter = 0;
            pInstance->SetData(DATA_EREDAR_TWINS_INTRO, IN_PROGRESS);
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (pInstance->GetData(DATA_EREDAR_TWINS_INTRO) == DONE)
            Scripted_NoMovementAI::MoveInLineOfSight(who);
    }

    void DamageMade(Unit* target, uint32 &damage, bool direct_damage, uint8 school_mask)
    {
        if(target->GetTypeId() == TYPEID_PLAYER && damage && me->isAlive())
        {
            if(school_mask == SPELL_SCHOOL_MASK_SHADOW)
            {
                SpellEntry* DarkTouched = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DARK_TOUCHED);
                if(target->HasAura(SPELL_FLAME_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }
                if(!target->HasAura(SPELL_DARK_FLAME) && !target->ToPlayer()->GetCooldownMgr().HasSpellIdCooldown(DarkTouched))
                {
                    target->CastSpell(target, SPELL_DARK_TOUCHED, true);
                    target->ToPlayer()->GetCooldownMgr().AddSpellIdCooldown(DarkTouched, 1000);
                }
            }
            if(school_mask == SPELL_SCHOOL_MASK_FIRE)
            {
                SpellEntry* FlameTouched = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(SPELL_FLAME_TOUCHED);
                if(target->HasAura(SPELL_DARK_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_DARK_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }
                if(!target->HasAura(SPELL_DARK_FLAME) && !target->ToPlayer()->GetCooldownMgr().HasSpellIdCooldown(FlameTouched))
                {
                    target->CastSpell(target, SPELL_FLAME_TOUCHED, true, NULL,NULL, me->GetGUID());
                    target->ToPlayer()->GetCooldownMgr().AddSpellIdCooldown(FlameTouched, 2800);
                }
            }
        }
    }

    void KilledUnit(Unit *victim)
    {
        if (roll_chance_i(20))
            DoScriptText(RAND(YELL_ALY_KILL_1, YELL_ALY_KILL_2), me);
    }

    void RemoveEncounterDebuffs()
    {
        Map *map = me->GetMap();
        if (!map->IsDungeon())
            return;

        Map::PlayerList const &PlayerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        for(i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            if (Player* i_pl = i->getSource())
            {
                i_pl->RemoveAurasDueToSpell(SPELL_DARK_TOUCHED);
                i_pl->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
            }
        }
        // despawn all flames from Blaze spell
        std::list<GameObject*> FlamesInRange;
        Hellground::AllGameObjectsWithEntryInGrid go_check(187366);
        Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(FlamesInRange, go_check);
        Cell::VisitGridObjects(me, go_search, 100.0f);
        if(!FlamesInRange.empty())
        {
            for (std::list<GameObject*>::const_iterator itr = FlamesInRange.begin(); itr != FlamesInRange.end(); ++itr)
                (*itr)->Delete();
        }
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance->GetData(DATA_SACROLASH) == DONE)
        {
            DoScriptText(YELL_ALY_DEAD, me);
            pInstance->SetData(DATA_EREDAR_TWINS_EVENT, DONE);
            RemoveEncounterDebuffs();
        }
        else
            me->SetLootRecipient(NULL);

        pInstance->SetData(DATA_ALYTHESS, DONE);
    }

    uint32 IntroStep(uint32 step)
    {
        Creature* pSacrolash = me->GetCreature(pInstance->GetData64(DATA_SACROLASH));
        if (!pSacrolash)
            return 0;

        switch (step)
        {
            case 0:
                return 0;
            case 1:
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                pSacrolash->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                DoScriptText(YELL_INTRO_SAC_1, pSacrolash);
                return 1000;
            case 2:
                DoScriptText(YELL_INTRO_ALY_2, me);
                return 1000;
            case 3:
                DoScriptText(YELL_INTRO_SAC_3, pSacrolash);
                return 2000;
            case 4:
                DoScriptText(YELL_INTRO_ALY_4, me);
                return 1000;
            case 5:
                DoScriptText(YELL_INTRO_SAC_5, pSacrolash);
                return 2000;
            case 6:
                DoScriptText(YELL_INTRO_ALY_6, me);
                return 1000;
            case 7:
                DoScriptText(YELL_INTRO_SAC_7, pSacrolash);
                return 3000;
            case 8:
                DoScriptText(YELL_INTRO_ALY_8, me);
                return 3000;
            case 9:
                pInstance->SetData(DATA_EREDAR_TWINS_INTRO, DONE);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                pSacrolash->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                return 900000;
            default:
                return 10000;
        }
    }

    void DoAction(const int32 action)
    {
        if (action == SISTER_DEATH)
            AddSpellToCastWithScriptText(SPELL_EMPOWER, CAST_SELF, YELL_SISTER_SACROLASH_DEAD);
    }

    // searches for one of 5 top threat targets from sisters' threat list, but not her main target
    Unit* GetConflagTarget()
    {
        if(Creature* Sacrolash = me->GetCreature(pInstance->GetData64(DATA_SACROLASH)))
        {
            
            Unit* target = Sacrolash->AI()->SelectUnit(SELECT_TARGET_TOPAGGRO, urand(1,5), 300.0f, true, Sacrolash->getVictimGUID());
            if(target && target->isAlive())
                return target;
        }
        return NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if(pInstance->GetData(DATA_FELMYST_EVENT) == DONE)
        {
            if (IntroStepCounter < 10)
            {
                if (IntroYellTimer.Expired(diff))
                {
                    IntroYellTimer = IntroStep(++IntroStepCounter);
                }
            }
        }

        if (!UpdateVictim())
            return;

        if (SpecialTimer.Expired(diff))
        {
            if (pInstance->GetData(DATA_SACROLASH) == DONE)
            {
                AddSpellToCastWithScriptText(SPELL_SHADOW_NOVA, CAST_RANDOM_WITHOUT_TANK, EMOTE_SHADOW_NOVA, false, true);
                SpecialTimer = urand(14000, 16000);
            }
            else
            {
                if(Unit* target = GetConflagTarget())
                    AddSpellToCastWithScriptText(target , SPELL_CONFLAGRATION, EMOTE_CONFLAGRATION, false, true);
                DoScriptText(YELL_CANFLAGRATION, me);
                SpecialTimer = urand(30000,35000);
            }
        }
        
        
        if (FlamesearTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_FLAME_SEAR, CAST_SELF);
            FlamesearTimer = 10000;
        }
        


        if (PyrogenicsTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_PYROGENICS, CAST_SELF);
            PyrogenicsTimer = 15000;
        }
        

        if (EnrageTimer.Expired(diff))
        {
            AddSpellToCastWithScriptText(SPELL_ENRAGE, CAST_SELF, YELL_BERSERK);
            EnrageTimer = 360000;
        }
        

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_alythess(Creature *_Creature)
{
    return new boss_alythessAI (_Creature);
};

struct mob_shadow_imageAI : public ScriptedAI
{
    mob_shadow_imageAI(Creature *c) : ScriptedAI(c) { pInstance = c->GetInstanceData(); }

    Timer_UnCheked ShadowfuryTimer;
    Timer_UnCheked DarkstrikeTimer;
    InstanceData *pInstance;

    void Reset()
    {
        ShadowfuryTimer.Reset(1500);
        DarkstrikeTimer.Reset(3000);
    }

    void AttackStart(Unit * target)
    {
        if (me->GetVictim())
            return;

        me->getThreatManager().addThreat(target, 100000.0f);
        ScriptedAI::AttackStart(target);
    }

    void IsSummonedBy(Unit *pSummoner)
    {
        ForceSpellCast(SPELL_IMAGE_VISUAL, CAST_SELF, INTERRUPT_AND_CAST_INSTANTLY);
        DoZoneInCombat();

        //if (Unit *pTarget = SelectUnit(SELECT_TARGET_FARTHEST, urand(0, 4), 400, true))
        if(Creature* Alythess = me->GetCreature(pInstance->GetData64(DATA_ALYTHESS)))
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 400, true, Alythess->getVictimGUID(), 10.0f))
                AttackStart(pTarget);
        }
    }

    void DamageMade(Unit* target, uint32 &damage, bool direct_damage, uint8 school_mask)
    {
        if(target->GetTypeId() == TYPEID_PLAYER && damage && school_mask == SPELL_SCHOOL_MASK_SHADOW)
        {
            SpellEntry* DarkTouched = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DARK_TOUCHED);
            if(target->HasAura(SPELL_FLAME_TOUCHED))
            {
                target->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
                target->CastSpell(target, SPELL_DARK_FLAME, true);
            }
            if(!target->HasAura(SPELL_DARK_FLAME) && !target->ToPlayer()->GetCooldownMgr().HasSpellIdCooldown(DarkTouched))
            {
                target->CastSpell(target, SPELL_DARK_TOUCHED, true);
                target->ToPlayer()->GetCooldownMgr().AddSpellIdCooldown(DarkTouched, 1000);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        
        if (ShadowfuryTimer.Expired(diff))
        {
            if (me->IsWithinMeleeRange(me->GetVictim()) && roll_chance_f(15))
            {
                AddSpellToCast(SPELL_SHADOW_FURY, CAST_NULL);
                ShadowfuryTimer = 5000;
            }
            else
                ShadowfuryTimer = 1500;
        }
        


        if (DarkstrikeTimer.Expired(diff))
        {
            if (!me->IsNonMeleeSpellCast(false))
            {
                //If we are within range melee the target
                if (me->IsWithinMeleeRange(me->GetVictim()))
                    AddSpellToCast(SPELL_DARK_STRIKE, CAST_TANK);
            }
            DarkstrikeTimer = 1000;
        }
        

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_mob_shadow_image(Creature *_Creature)
{
    return new mob_shadow_imageAI (_Creature);
};

void AddSC_boss_eredar_twins()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_sacrolash";
    newscript->GetAI = &GetAI_boss_sacrolash;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="boss_alythess";
    newscript->GetAI = &GetAI_boss_alythess;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shadow_image";
    newscript->GetAI = &GetAI_mob_shadow_image;
    newscript->RegisterSelf();
}
