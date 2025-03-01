// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Kalecgos
SD%Complete: 98
SDComment: check cooldowns, final debugging
SDCategory: Sunwell_Plateau
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"

enum Quotes
{
    //Kalecgos dragon form
    SAY_EVIL_AGGRO          = -1580000,
    SAY_EVIL_SPELL1         = -1580001,
    SAY_EVIL_SPELL2         = -1580002,
    SAY_EVIL_SLAY1          = -1580003,
    SAY_EVIL_SLAY2          = -1580004,
    SAY_EVIL_ENRAGE         = -1580005,

    //Kalecgos humanoid form
    SAY_GOOD_AGGRO          = -1580006,
    SAY_GOOD_NEAR_DEATH     = -1580007,
    SAY_GOOD_NEAR_DEATH2    = -1580008,
    SAY_GOOD_PLRWIN         = -1580009,
    SAY_GOOD_GREET1         = -1579992,
    SAY_GOOD_GREET2         = -1579993,
    SAY_GOOD_GREET3         = -1579994,

    //Shattrowar
    SAY_SATH_AGGRO          = -1580010,
    SAY_SATH_DEATH          = -1580011,
    SAY_SATH_SPELL1         = -1580012,
    SAY_SATH_SPELL2         = -1580013,
    SAY_SATH_SLAY1          = -1580014,
    SAY_SATH_SLAY2          = -1580015,
    SAY_SATH_ENRAGE         = -1580016,

    //Enrage emotes
    EMOTE_KALECGOS_ENRAGE   = -1579990,
    EMOTE_SATHROVARR_ENRAGE = -1579991
};

enum SpellIds
{
    AURA_SPECTRAL_EXHAUSTION    =   44867,
    AURA_SPECTRAL_REALM         =   46021,
    AURA_SPECTRAL_INVISIBILITY  =   44801,  // aura in creature_template_addon
    AURA_DEMONIC_VISUAL         =   44800,  // aura in creature_template_addon

    SPELL_SPECTRAL_BLAST        =   44869,
    SPELL_TELEPORT_SPECTRAL     =   46019,  // linked in DB
    SPELL_TELEPORT_NORMAL       =   46020,  // linked in DB
    SPELL_ARCANE_BUFFET         =   45018,
    SPELL_FROST_BREATH          =   44799,
    SPELL_TAIL_LASH             =   45122,

    SPELL_BANISH                =   44836,
    SPELL_TRANSFORM_KALEC       =   44670,  //not used here?
    SPELL_ENRAGE                =   44807,

    SPELL_CORRUPTION_STRIKE     =   45029,
    SPELL_AGONY_CURSE           =   45032,
    SPELL_AGONY_CURSE2          =   45034,
    SPELL_SHADOW_BOLT           =   45031,

    SPELL_HEROIC_STRIKE         =   45026,
    SPELL_REVITALIZE            =   45027
};

enum Phases
{
    PHASE_COMBAT = 10,          // before any boss is down to 10%
    PHASE_ENRAGE = 11,          // before any boss is down to 1%
    PHASE_BANISH = 12,          // after one boss has been banished
    PHASE_DOUBLE_BANISH = 13,   // after both bosses has been banished
    PHASE_KALEC_DEAD = 14       // when kalec is dead
};

enum Creatures
{
    MOB_KALECGOS    =  24850,
    MOB_KALEC       =  24891,
    MOB_SATHROVARR  =  24892
};

#define GO_FAILED   -1200291

static float FlyCoord[][3] = 
{
    {1679, 900, 82},
    {1668, 690, 161}
};

#define CENTER_X    1705
#define CENTER_Y    930
#define RADIUS      30

#define DRAGON_REALM_Z  53.079
#define DEMON_REALM_Z   -74.558

uint32 WildMagic[]= { 44978, 45001, 45002, 45004, 45006, 45010 };

struct boss_kalecgosAI : public ScriptedAI
{
    boss_kalecgosAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceData();
        me->SetAggroRange(26.0f);
        me->GetPosition(wLoc);
    }

    ScriptedInstance *instance;

    Timer_UnCheked ArcaneBuffetTimer;
    Timer_UnCheked FrostBreathTimer;
    Timer_UnCheked WildMagicTimer;
    Timer_UnCheked SpectralBlastTimer;
    Timer_UnCheked TailLashTimer;
    Timer_UnCheked CheckTimer;
    Timer_UnCheked TalkTimer;
    uint32 TalkSequence;
    Timer_UnCheked ResetTimer;
    WorldLocation wLoc;

    bool isFriendly;
    bool isEnraged;
    bool isBanished;
    bool CanCastBlast;
    bool TalkingDone;

    Timer stateCheckTimer;

    void Reset()
    {
        me->setFaction(14);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE, false);
        me->SetLevitate(false);
        me->RemoveUnitMovementFlag(MOVEFLAG_ONTRANSPORT);
        me->SetStandState(UNIT_STAND_STATE_SLEEP);

        ArcaneBuffetTimer.Reset(8000);
        FrostBreathTimer.Reset(15000);
        WildMagicTimer.Reset(10000);
        TailLashTimer.Reset(25000);
        ResetTimer = 0;
        SpectralBlastTimer.Reset(20000 + (rand() % 5000));
        CheckTimer.Reset(1000);

        stateCheckTimer.Reset(2000);

        TalkTimer = 0;
        TalkSequence = 0;
        isFriendly = false;
        TalkingDone = false;
        isEnraged = false;
        isBanished = false;
        CanCastBlast = true;

        instance->SetData(DATA_KALECGOS_EVENT, NOT_STARTED);
        instance->SetData(DATA_KALECGOS_PHASE, NOT_STARTED);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (damage >= me->GetHealth() && done_by != me)
            damage = 0;
    }

    void EnterCombat(Unit* who)
    {
        me->SetStandState(UNIT_STAND_STATE_STAND);
        DoScriptText(SAY_EVIL_AGGRO, me);

        DoZoneInCombat();

        instance->SetData(DATA_KALECGOS_EVENT, IN_PROGRESS);
        instance->SetData(DATA_KALECGOS_PHASE, PHASE_COMBAT);
    }

    void EnterEvadeMode()
    {
        CreatureAI::EnterEvadeMode();
        TalkSequence = 0;

        me->setFaction(35); //friendly for when invisible
        me->SetVisibility(VISIBILITY_OFF);
        ResetTimer = 20000;
    }

    void KilledUnit(Unit *victim)
    {
        if (roll_chance_f(10.0))
            DoScriptText(RAND(SAY_EVIL_SLAY1, SAY_EVIL_SLAY2), me);
    }

    void GoodEnding()
    {
        switch (TalkSequence)
        {
        case 1:
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->setFaction(35);
            TalkTimer = 8000;
            break;
        case 2:
            me->SetOrientation(M_PI);  //? check this out
            me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
            me->SetLevitate(true);
            me->setHover(true);
            me->SendHeartBeat();
            TalkTimer = 3000;
            break;
        case 3:
            float x, y, z;
            me->GetPosition(x, y, z);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(0,x,y,z+7);
            TalkTimer = 4000;
            break;
        case 4:
            DoScriptText(SAY_GOOD_PLRWIN, me);
            TalkTimer = 10000;
            break;
        case 5:
            DoScriptText(RAND(SAY_GOOD_GREET1, SAY_GOOD_GREET2, SAY_GOOD_GREET3), me);
            me->GetMotionMaster()->MovePoint(1,FlyCoord[0][0],FlyCoord[0][1],FlyCoord[0][2]);
            TalkTimer = 7000;
            break;
        case 6:
            me->GetMotionMaster()->MovePoint(2, FlyCoord[1][0],FlyCoord[1][1],FlyCoord[1][2]);
            TalkTimer = 20000;
            break;
        case 7:
            me->SetVisibility(VISIBILITY_OFF);
            TalkingDone = true;
            TalkTimer = 0;
            break;
        default:
            break;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!TalkTimer.GetInterval() && !ResetTimer.GetInterval())
            CreatureAI::MoveInLineOfSight(who);
    }

    void BadEnding()
    {
        switch(TalkSequence)
        {
            case 1:
                DoScriptText(SAY_EVIL_ENRAGE, me);
                TalkTimer = 3000;
                break;
            case 2:
                me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                me->SetLevitate(true);
                me->setHover(true);
                me->SendHeartBeat();
                TalkTimer = 3000;
                break;
            case 3:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0,FlyCoord[1][0],FlyCoord[1][1],FlyCoord[1][2]);
                TalkTimer = 10000;
                break;
            case 4:
                EnterEvadeMode();
                if (Creature* Sathrovarr = Creature::GetCreature(*me, instance->GetData64(DATA_SATHROVARR)))
                    Sathrovarr->AI()->EnterEvadeMode();
                
                break;
            default:
                break;
        }
    }

    bool EncounterInProgressCheck()
    {
        if (me->IsInEvadeMode())
            return false;

        if (me->GetMap()->GetAlivePlayersCountExceptGMs() == 0)
        {
            EnterEvadeMode();
            return false;
        }
        return true;
    }

    void UpdateAI(const uint32 diff)
    {      
        if (TalkingDone)
            return;

        if (stateCheckTimer.Expired(diff))
        {
            stateCheckTimer = 2000;
            if (!EncounterInProgressCheck())
                return;
        }

        if (ResetTimer.GetInterval())
        {
            if (ResetTimer.Expired(diff))
            {
                //!instance->GetData(DATA_OPEN_PATH)
                if (instance->GetData(DATA_START_TRASH_EVENT) < 30)
                    return;
                
                ResetTimer = 0;
                me->setFaction(16);     //aggresive
                me->SetVisibility(VISIBILITY_ON);
            }
            return;
        }
        else if (TalkTimer.GetInterval())
        {
            if (!TalkSequence)
            {
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE + UNIT_FLAG_NOT_SELECTABLE);
                me->InterruptNonMeleeSpells(true);
                me->RemoveAllAurasNotCreatureAddon();
                me->DeleteThreatList();
                me->CombatStop();
                ClearCastQueue();
                TalkSequence++;
            }

            if (TalkTimer.Expired(diff))
            {
                if (isFriendly)
                    GoodEnding();
                else
                    BadEnding();

                TalkSequence++;
            }
            
        }
        else
        {
            // be sure to not attack players in spectral realm
            if (me->GetVictim() && me->GetVictim()->HasAura(AURA_SPECTRAL_REALM, 0))
            {
                // if player in spectral realm is on top of threat list either
                // he has taunted us or there is no alive player outside spectral realm
                me->RemoveSpellsCausingAura(SPELL_AURA_MOD_TAUNT);
                if (!UpdateVictim())
                    return;

                if (me->GetVictim()->HasAura(AURA_SPECTRAL_REALM, 0))
                {
                    EnterEvadeMode();
                    return;
                }
            }

            // if still having victim with aura, drop some threat
            if (me->GetVictim() && me->GetVictim()->HasAura(AURA_SPECTRAL_REALM, 0))
                me->getThreatManager().modifyThreatPercent(me->GetVictim(), -10);

            // various checks + interaction with sathrovarr
            if (CheckTimer.Expired(diff))
            {
                if (!me->IsWithinDistInMap(&wLoc, 30))
                    EnterEvadeMode();

                if (instance && instance->GetData(DATA_KALECGOS_PHASE) == PHASE_ENRAGE && !isEnraged)
                {
                    me->CastSpell(me, SPELL_ENRAGE, true);
                    isEnraged = true;
                }

                if (!isEnraged && HealthBelowPct(10))
                {
                    instance->SetData(DATA_KALECGOS_PHASE, PHASE_ENRAGE);

                    DoScriptText(EMOTE_SATHROVARR_ENRAGE, me);
                    if (Unit* pSathrovarr = Unit::GetUnit(*me, instance->GetData64(DATA_SATHROVARR)))
                        DoScriptText(SAY_SATH_ENRAGE, pSathrovarr);

                    me->CastSpell(me, SPELL_ENRAGE, true);  // this will affect also sathrovarr
                    isEnraged = true;
                }

                if (!isBanished && HealthBelowPct(1))
                {
                    ForceSpellCast(me, SPELL_BANISH);
                    isBanished = true;
                    if (instance)
                    {
                        if (instance->GetData(DATA_KALECGOS_PHASE) == PHASE_BANISH)
                            instance->SetData(DATA_KALECGOS_PHASE, PHASE_DOUBLE_BANISH);
                        else
                            instance->SetData(DATA_KALECGOS_PHASE, PHASE_BANISH);
                    }
                }

                if (instance->GetData(DATA_KALECGOS_PHASE) == PHASE_KALEC_DEAD)
                {
                    TalkTimer = 1;
                    TalkSequence = 0;
                    isFriendly = false;
                    return;
                }
                if (instance->GetData(DATA_KALECGOS_EVENT) == DONE && !TalkingDone)
                {
                    TalkTimer = 1;
                    TalkSequence = 0;
                    isFriendly = true;
                    return;
                }
                CheckTimer = 1000;

                if (me->IsInCombat())
                    DoZoneInCombat();
            }

            if (!UpdateVictim())
                return;

            // cast spells
            if (ArcaneBuffetTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_ARCANE_BUFFET, CAST_SELF);
                if (roll_chance_f(20.0))
                    DoScriptText(RAND(SAY_EVIL_SPELL1, SAY_EVIL_SPELL2), me);

                ArcaneBuffetTimer = 8000;
            }

            if (FrostBreathTimer.Expired(diff))
            {
                if (roll_chance_f(20.0))
                    DoScriptText(RAND(SAY_EVIL_SPELL1, SAY_EVIL_SPELL2), me);

                AddSpellToCast(SPELL_FROST_BREATH, CAST_SELF);
                FrostBreathTimer = 15000;
            }

            if (TailLashTimer.Expired(diff))
            {
                if (roll_chance_f(20.0))
                    DoScriptText(RAND(SAY_EVIL_SPELL1, SAY_EVIL_SPELL2), me);

                AddSpellToCast(SPELL_TAIL_LASH, CAST_SELF);
                TailLashTimer = 15000;
            }

            if (WildMagicTimer.Expired(diff))
            {
                AddSpellToCast(WildMagic[rand()%6], CAST_SELF);
                WildMagicTimer = 20000;
            }

            if (CanCastBlast)
            {
                if (SpectralBlastTimer.Expired(diff))
                {
                    AddSpellToCast(SPELL_SPECTRAL_BLAST, CAST_SELF);
                    SpectralBlastTimer = 20000+(rand()%5000);
                }
            }
            CastNextSpellIfAnyAndReady();
            DoMeleeAttackIfReady();
        }
    }
};

struct boss_sathrovarrAI : public ScriptedAI
{
    boss_sathrovarrAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceData();
        KalecGUID = 0;
    }

    ScriptedInstance *instance;

    Timer_UnCheked CorruptionStrikeTimer;
    Timer_UnCheked AgonyCurseTimer;
    Timer_UnCheked ShadowBoltTimer;
    Timer_UnCheked CheckTimer;

    uint64 KalecGUID;
    bool isEnraged;
    bool isBanished;

    void Reset()
    {
        ShadowBoltTimer.Reset(7000 + rand() % 3 * 1000);
        AgonyCurseTimer.Reset(urand(20000, 35000));
        CorruptionStrikeTimer.Reset(13000);
        CheckTimer.Reset(1000);
        isEnraged = false;
        isBanished = false;
        me->setActive(true);

        instance->SetData(DATA_KALECGOS_EVENT, NOT_STARTED);
    }

    void EnterEvadeMode()
    {
        if (KalecGUID)
        {
            if (Unit* Kalec = Unit::GetUnit(*me, KalecGUID))
                Kalec->setDeathState(JUST_DIED);

            KalecGUID = 0;
        }
        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* who)
    {
        float x, y, z, ori;
        me->GetNearPoint(x, y, z, 0, 6, me->GetOrientationTo(me));
        ori = me->GetOrientationTo(x, y);
        Creature* Kalec = me->SummonCreature(MOB_KALEC, x, y, z, ori, TEMPSUMMON_CORPSE_DESPAWN, 0);
        if (Kalec)
        {
            KalecGUID = Kalec->GetGUID();
            AttackStart(Kalec);
            Kalec->Attack(me, true);
            Kalec->GetMotionMaster()->MoveChase(me);
            me->AddThreat(Kalec, 100.0f);
        }
        DoScriptText(SAY_SATH_AGGRO, me);
        DoZoneInCombat();   // put all players into threatlist
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (damage >= me->GetHealth() && done_by != me)
            damage = 0;
        if (!me->GetLootRecipient() && done_by->GetTypeId() == TYPEID_PLAYER)
            me->SetLootRecipient(done_by);
    }

    void KilledUnit(Unit *target)
    {
        if (target->GetGUID() == KalecGUID)
        {
            instance->SetData(DATA_KALECGOS_EVENT, NOT_STARTED);
            instance->SetData(DATA_KALECGOS_PHASE, PHASE_KALEC_DEAD);

            TeleportAllPlayersBack();
            EnterEvadeMode();
            return;
        }

        if (roll_chance_f(10.0))
            DoScriptText(RAND(SAY_SATH_SLAY1, SAY_SATH_SLAY2), me);
    }

    void JustDied(Unit *killer)
    {
        DoScriptText(SAY_SATH_DEATH, me);
        DoTeleportTo(me->GetPositionX(), me->GetPositionY(), DRAGON_REALM_Z, 0);

        instance->SetData(DATA_KALECGOS_EVENT, DONE);
        TeleportAllPlayersBack();  // must be called after setting encounter done
    }

    void TeleportAllPlayersBack()
    {
        Map *map = me->GetMap();
        if (!map->IsDungeon())
            return;

        Map::PlayerList const &PlayerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        for(i = PlayerList.begin(); i != PlayerList.end(); ++i)
            if (Player* i_pl = i->getSource())
            {
                i_pl->RemoveAurasDueToSpell(AURA_SPECTRAL_REALM);
                i_pl->RemoveAurasDueToSpell(SPELL_AGONY_CURSE);
                i_pl->RemoveAurasDueToSpell(SPELL_AGONY_CURSE2);
            }
    }

    void UpdateAI(const uint32 diff)
    {
      
        // interaction with kalecgos
        if (CheckTimer.Expired(diff))
        {
            // should not leave Inner Veil
            if (me->GetPositionZ() > -60)
                me->GetMap()->CreatureRelocation(me, me->GetPositionX(), me->GetPositionY(), DEMON_REALM_Z, me->GetOrientation());

            if (instance && instance->GetData(DATA_KALECGOS_PHASE) == PHASE_ENRAGE)
            {
                DoCast(me, SPELL_ENRAGE, true);
                isEnraged = true;
            }

            if (!isEnraged && HealthBelowPct(10))
            {
                DoCast(me, SPELL_ENRAGE, true); // this will cast enrage also on kalecgos
                DoScriptText(SAY_SATH_ENRAGE, me);
                DoScriptText(EMOTE_KALECGOS_ENRAGE, me);
                instance->SetData(DATA_KALECGOS_PHASE, PHASE_ENRAGE);
                isEnraged = true;
            }

            if (!isBanished && HealthBelowPct(1))
            {
                if (instance->GetData(DATA_KALECGOS_PHASE) == PHASE_BANISH)
                {
                    me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    return;
                }
                else
                {
                    ForceSpellCast(me, SPELL_BANISH);
                    isBanished = true;
                    if (Creature* Kalecgos = Creature::GetCreature(*me, instance->GetData64(DATA_KALECGOS_DRAGON)))
                        ((boss_kalecgosAI*)Kalecgos->AI())->CanCastBlast = false;
                    instance->SetData(DATA_KALECGOS_PHASE, PHASE_BANISH);
                }
            }

            if (isBanished && instance->GetData(DATA_KALECGOS_PHASE) == PHASE_DOUBLE_BANISH)
            {
                me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                return;
            }

            if (instance->GetData(DATA_KALECGOS_EVENT) == NOT_STARTED) // kalecgos evaded
            {
                TeleportAllPlayersBack();
                EnterEvadeMode();
                return;
            }
            if (me->IsInCombat())
                DoZoneInCombat();

            CheckTimer = 1000;
        }
        

        // to be tested
        if (me->GetVictim() && (!me->GetVictim()->HasAura(AURA_SPECTRAL_REALM)  || me->GetVictim()->GetPositionZ() > -50)  && !(me->GetVictim()->GetEntry() == MOB_KALEC))
            DoModifyThreatPercent(me->GetVictim(), -100);

        // be sure to attack only players in spectral realm
        if (me->GetVictim() && me->GetVictim()->HasAura(AURA_SPECTRAL_EXHAUSTION))
        {
            me->RemoveSpellsCausingAura(SPELL_AURA_MOD_TAUNT);
            if (!UpdateVictim())
                return;
        }

        if (!UpdateVictim())
            return;

        // cast spells
        if (ShadowBoltTimer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f, true);
            if (target)
                AddSpellToCast(target, SPELL_SHADOW_BOLT);

            if (roll_chance_f(10.0))
                DoScriptText(SAY_SATH_SPELL1, me);

            ShadowBoltTimer = 7000+(rand()%3000);
        }

        if (AgonyCurseTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_AGONY_CURSE, CAST_SELF);
            AgonyCurseTimer = 35000;
        }

        if (CorruptionStrikeTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_CORRUPTION_STRIKE);
            if (roll_chance_f(10.0))
                DoScriptText(SAY_SATH_SPELL2, me);

            CorruptionStrikeTimer = 13000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct boss_kalecAI : public ScriptedAI
{
    ScriptedInstance *instance;

    Timer_UnCheked RevitalizeTimer;
    Timer_UnCheked HeroicStrikeTimer;
    Timer_UnCheked CheckTimer;
    Timer_UnCheked YellTimer;
    uint32 YellSequence;

    bool isEnraged;

    uint64 SathGUID;

    boss_kalecAI(Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceData();
        SathGUID = 0;
    }

    void Reset()
    {
        RevitalizeTimer.Reset(5000);
        HeroicStrikeTimer.Reset(3000);
        CheckTimer.Reset(1000);
        YellTimer.Reset(10000);
        YellSequence = 0;
        me->setActive(true);

        isEnraged = false;

        SathGUID = instance->GetData64(DATA_SATHROVARR);
    }

    Unit* SelectUnitToRevitalize()
    {
        std::list<Unit*> RealmUnitList;
        std::list<HostileReference*>& ThreatList = me->getThreatManager().getThreatList();
        RealmUnitList.clear();

        if (ThreatList.empty())
            return NULL;

        for(std::list<HostileReference*>::iterator i = ThreatList.begin() ; i!=ThreatList.end() ; ++i)
        {
            Unit* target = Unit::GetUnit(*me, (*i)->getUnitGuid());
            // only castable on players in spectral realm that have mana pool and are not revitalized yet
            if (target && (!target->HasAura(AURA_SPECTRAL_REALM, 0) || !target->HasAura(SPELL_REVITALIZE, 0)) && target->GetPower(POWER_MANA))
                RealmUnitList.push_back(target);
        }

        if (RealmUnitList.empty())
            return NULL;

        std::list<Unit*>::iterator itr = RealmUnitList.begin();
        advance(itr, (rand()%RealmUnitList.size()));
        return *itr;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetGUID() != SathGUID)
            damage = 0;
        else if (isEnraged)
            damage *= 3;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        
        if (YellTimer.Expired(diff))
        {
            switch(YellSequence)
            {
            case 0:
                DoScriptText(SAY_GOOD_AGGRO, me);
                YellSequence++;
                break;
            case 1:
                if (HealthBelowPct(50))
                {
                    DoScriptText(SAY_GOOD_NEAR_DEATH, me);
                    YellSequence++;
                }
                break;
            case 2:
                if (HealthBelowPct(10))
                {
                    DoScriptText(SAY_GOOD_NEAR_DEATH2, me);
                    YellSequence++;
                }
                break;
            default:
                break;
            }
            YellTimer = 5000;
        }
        

      
        if (CheckTimer.Expired(diff))
        {
            if (instance && instance->GetData(DATA_KALECGOS_PHASE) == PHASE_ENRAGE)
                isEnraged = true;

            // should not leave Inner Veil
            if (me->GetPositionZ() > -60)
                me->GetMap()->CreatureRelocation(me, me->GetPositionX(), me->GetPositionY(), DEMON_REALM_Z, me->GetOrientation());

            CheckTimer = 1000;
        }

        if (RevitalizeTimer.Expired(diff))
        {
            if (Unit* target = SelectUnitToRevitalize())
                AddSpellToCast(target, SPELL_REVITALIZE, false, true);
            RevitalizeTimer = 7000;
        }

        if (HeroicStrikeTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_HEROIC_STRIKE);
            HeroicStrikeTimer = 2000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

bool GOkalecgos_teleporter(Player *player, GameObject* _GO)
{
    if (player->HasAura(AURA_SPECTRAL_EXHAUSTION, 0))
    {
        player->GetSession()->SendNotification(-1200291);
        return true;
    }
    return false;
}

CreatureAI* GetAI_boss_kalecgos(Creature *_Creature)
{
    return new boss_kalecgosAI (_Creature);
}

CreatureAI* GetAI_boss_Sathrovarr(Creature *_Creature)
{
    return new boss_sathrovarrAI (_Creature);
}

CreatureAI* GetAI_boss_kalec(Creature *_Creature)
{
    return new boss_kalecAI (_Creature);
}

void AddSC_boss_kalecgos()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kalecgos";
    newscript->GetAI = &GetAI_boss_kalecgos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_sathrovarr";
    newscript->GetAI = &GetAI_boss_Sathrovarr;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_kalec";
    newscript->GetAI = &GetAI_boss_kalec;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="kalecgos_teleporter";
    newscript->pGOUse = &GOkalecgos_teleporter;
    newscript->RegisterSelf();
}
