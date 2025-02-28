// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
// script is owned by DeathSide, Trentone
#define Coord_X 0
#define Coord_Y 0
#define Coord_Z 0
#define Orientation 0
#define MOB_SKULL_OF_DEATH 0
#define SPELL_BIG_VOID_ZONE_STARTER 0
#define MOB_VOID_SAVIOR 0
#define SPELL_SELF_CONTROL 0 // Only stops movement
#define DIST_ROOM 0.0
#define SPELL_BURN_STARTER 0
#define SPELL_INNER_FIRE 0
#define SPELL_SHACKLES_OF_DARKNESS 0
#define SPELL_SUMMON_SHACKLES_OF_DARKNESS 0
#define MOB_SHACKLES_OF_DARKNESS 0
#define SPELL_SHACKLES_VISUAL 5555
#define MOB_CENTER_OF_ROOM 0
#define SPELL_DEATH_BOLT 0
#define SPELL_SOUL_VISUAL 5555
#define SPELL_SOUL_TERROR 5555
#define SPELL_DEATH_FORM 0
#define SPELL_TELEPORT_VISUAL 0
#define VOID_ZONE 0
#define SPELL_SHADOW_INSIDE 5555 // takoi krug v 4are kogda cast. Kak budto shadow shock tipa
#define SPELL_ANGEL_SAVIOR 0
#define SPELL_SHADOW_EXPLOSION 0
#define SPELL_ENDARKNESS 0

struct deathside_mob_shackles_of_darknessAI : public ScriptedAI
{
    deathside_mob_shackles_of_darknessAI(Creature *c) : ScriptedAI(c) {}

    uint64 ShacklesTargetGUID;

    void Reset()
    {
        ShacklesTargetGUID = 0;
    }

    void EnterCombat(Unit* /*who*/) {}
    void AttackStart(Unit* /*who*/) {}
    void MoveInLineOfSight(Unit* /*who*/) {}

    void JustDied(Unit * /*killer*/)
    {
        if (ShacklesTargetGUID)
        {
            Unit* Sacrifice = Unit::GetUnit((*me),ShacklesTargetGUID);
            if (Sacrifice)
                Sacrifice->RemoveAurasDueToSpell(SPELL_SHACKLES_OF_DARKNESS);
        }
    }
};

struct deathside_boss_soul_blighterAI : public ScriptedAI
{
    deathside_boss_soul_blighterAI(Creature *c) : ScriptedAI(c) {}
    
    uint32 TeleportDamageDealerToVoidZoneTimer;
    uint32 SkullsOfDeathSummonTimer;
    uint32 BigVoidZoneTimer;
    uint32 FelFireTimer;
    uint32 InnerBurnTimer;
    uint32 ShacklesOfDarknessTimer;
    uint32 RoomRangeCheckTimer;
    uint32 SoulTerrorTimer;
    uint8 DeathCounter;
    uint32 DeathTimer;
    uint32 VoidZoneSplashTimer;
    uint32 ShadowExplosionTimer;
    uint32 TankEndarkenedTimer;

    void Reset()
    {
        TeleportDamageDealerToVoidZoneTimer = 0;
        SkullsOfDeathSummonTimer = 0;
        BigVoidZoneTimer = 0;
        FelFireTimer = 0;
        InnerBurnTimer = 0;
        ShacklesOfDarknessTimer = 0;
        RoomRangeCheckTimer = 0;
        SoulTerrorTimer = 0;
        DeathCounter = 0;
        DeathTimer = 0;
        VoidZoneSplashTimer = 0;
        ShadowExplosionTimer = 0;
        TankEndarkenedTimer = 0;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
    }

    void EnterCombat(Unit * /*who*/)
    {
        
    }

    void KilledUnit(Unit * victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DeathCounter++;
        victim->CastSpell(me, SPELL_SOUL_VISUAL, true);
    }

    void JustDied(Unit* Killer)
    {

    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() || me->HasAura(SPELL_SELF_CONTROL, 0))
            return;

        if (RoomRangeCheckTimer <= diff)
        {
            if (!FindCreature(MOB_CENTER_OF_ROOM, DIST_ROOM, me))
                DoCast(me->GetVictim(), SPELL_DEATH_BOLT, false); // Death Bolt is a shadow bolt with 100k damage and not possible to reflect/immune/resist
            RoomRangeCheckTimer = 10000;
        }
        else RoomRangeCheckTimer -= diff;

        if (TeleportDamageDealerToVoidZoneTimer <= diff)
        {
            std::list<Unit*> pTargets;
            SelectUnitList(pTargets, 25, SELECT_TARGET_RANDOM, 50, true);
            for (std::list<Unit*>::const_iterator i = pTargets.begin(); i != pTargets.end(); ++i)
                if ((*i)->isAlive() && me->GetVictim() != (*i) && (*i)->GetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS)/2 < (*i)->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS)) // Choosing damage dealer
                {
                    // here will be switch for random void zones
                    DoTeleportPlayer((*i), Coord_X, Coord_Y, Coord_Z, Orientation);
                    break;
                }
            TeleportDamageDealerToVoidZoneTimer = 500000000;
        }
        else TeleportDamageDealerToVoidZoneTimer -= diff;

        if (SkullsOfDeathSummonTimer <= diff)
        {
            Creature* Skull = NULL;

            Skull = me->SummonCreature(MOB_SKULL_OF_DEATH, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
            if (Skull)
                Skull->AI()->AttackStart(me->GetVictim());
            Skull = me->SummonCreature(MOB_SKULL_OF_DEATH, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
            if (Skull)
                Skull->AI()->AttackStart(me->GetVictim());
            Skull = me->SummonCreature(MOB_SKULL_OF_DEATH, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
            if (Skull)
                Skull->AI()->AttackStart(me->GetVictim());
            Skull = me->SummonCreature(MOB_SKULL_OF_DEATH, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
            if (Skull)
                Skull->AI()->AttackStart(me->GetVictim());

            DoCast(me, SPELL_TELEPORT_VISUAL, true); // Teleport visual effect
            me->NearTeleportTo(Coord_X, Coord_Y, Coord_Z, Orientation); // Teleport to center of the room
            DoCast(me, SPELL_SELF_CONTROL, true);            
            SkullsOfDeathSummonTimer = 50000;
        }
        else SkullsOfDeathSummonTimer -= diff;

        if (BigVoidZoneTimer <= diff)
        {
            DoCast(me, SPELL_BIG_VOID_ZONE_STARTER, true);
            me->SummonCreature(MOB_VOID_SAVIOR, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 48000);
            me->SummonCreature(MOB_VOID_SAVIOR, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 48000);
            me->SummonCreature(MOB_VOID_SAVIOR, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 48000);
            me->SummonCreature(MOB_VOID_SAVIOR, Coord_X, Coord_Y, Coord_Z, Orientation, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 48000);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            DoCast(me, SPELL_TELEPORT_VISUAL, true); // Teleport visual effect
            me->NearTeleportTo(Coord_X, Coord_Y, Coord_Z, Orientation); // Teleport to center of the room
            DoCast(me, SPELL_SELF_CONTROL, true);            
            BigVoidZoneTimer = 5000000;
        }
        else BigVoidZoneTimer -= diff;

        if (FelFireTimer <= diff)
        {
            Unit* FelFireTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, DIST_ROOM, true);
            if (FelFireTarget)
            {
                DoCast(FelFireTarget, SPELL_BURN_STARTER, false);
                FelFireTimer = 5000000;
            }            
        }
        else FelFireTimer -= diff;;

        if (InnerBurnTimer <= diff)
        {
            Unit* InnerFireTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, DIST_ROOM, true);
            if (InnerFireTarget)
            {
                DoCast(InnerFireTarget, SPELL_INNER_FIRE, false);
                InnerBurnTimer = 5000000;
            }            
        }
        else InnerBurnTimer -= diff;

        if (ShacklesOfDarknessTimer <= diff)
        {
            Unit *ShacklesOfDarknessTarget = NULL;
            for (uint8 i = 0; i < 25; i++)
            {
                ShacklesOfDarknessTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, i);
                if (ShacklesOfDarknessTarget && ShacklesOfDarknessTarget->isAlive() && ShacklesOfDarknessTarget->GetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS)/2 > ShacklesOfDarknessTarget->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS))
                {
                    // can be grabbed from terestian illhoof
                    DoCast(ShacklesOfDarknessTarget, SPELL_SHACKLES_OF_DARKNESS, true);
                    DoCast(ShacklesOfDarknessTarget, SPELL_SUMMON_SHACKLES_OF_DARKNESS, true);
                    break;
                }
            }
            if (Unit* Shackles = FindCreature(MOB_SHACKLES_OF_DARKNESS, DIST_ROOM, me))
            {
                ((deathside_mob_shackles_of_darknessAI*)Shackles->ToCreature()->AI())->ShacklesTargetGUID = ShacklesOfDarknessTarget->GetGUID();
                Shackles->CastSpell(Shackles, SPELL_SHACKLES_VISUAL, true);
            }
            ShacklesOfDarknessTimer = 50000000;
        }
        else ShacklesOfDarknessTimer -= diff;

        if (SoulTerrorTimer <= diff)
        {
            if (DeathCounter != 0)
            {
                Unit* SoulTerrorTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, DIST_ROOM, true); // non-tank
                if (SoulTerrorTarget)
                    SoulTerrorTimer = 500000000/DeathCounter;
            }
            else
                SoulTerrorTimer = 500000000;
        }
        else SoulTerrorTimer -= diff;

        if (DeathTimer <= diff)
        {
            DoCast(me, SPELL_DEATH_FORM, true); // When Theldelor get hit by anything damage spellcast or damage (NOT DoT's) he casts back a spells that reduces damage done by players. Darkness visual
            DeathTimer = 50000000;
        }
        else DeathTimer -= diff;

        if (VoidZoneSplashTimer <= diff)
        {
            int32 PlayersInVoidZone = 0;
            std::list<Unit*> pTargets;
            Map* pMap = me->GetMap();
            if (pMap)
            {
                Map::PlayerList const &PlayerList = pMap->GetPlayers();
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    if (Player* Target = i->getSource())
                        if (Target->isAlive() && Target->HasAura(VOID_ZONE, 0)) // Void zone
                        {
                            pTargets.push_back(Target);
                            PlayersInVoidZone++;
                        }
            }
            if (PlayersInVoidZone != 0)
            {
                PlayersInVoidZone = 1000/PlayersInVoidZone;
                for (std::list<Unit*>::const_iterator i = pTargets.begin(); i != pTargets.end(); ++i)
                {
                    me->CastCustomSpell((*i), SPELL_SHADOW_INSIDE, &PlayersInVoidZone, NULL, NULL, false);
                }
            }
            else
            {
                if (Unit* Target = SelectUnit(SELECT_TARGET_RANDOM, 0, DIST_ROOM, true))
                    DoCast(Target, SPELL_SHADOW_INSIDE, false);
            }
            VoidZoneSplashTimer = 50000000;
        }
        else VoidZoneSplashTimer -= diff;

        if (ShadowExplosionTimer <= diff)
        {
            DoCast(me->GetVictim(), SPELL_ANGEL_SAVIOR, false);
            DoCast(me, SPELL_SHADOW_EXPLOSION, true);
            ShadowExplosionTimer = 50000000;
        }
        else ShadowExplosionTimer -= diff;

        if (TankEndarkenedTimer <= diff)
        {
            DoCast(me->GetVictim(), SPELL_ENDARKNESS, false);
            TankEndarkenedTimer = 500000000;
        }
        else TankEndarkenedTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_deathside_mob_shackles_of_darkness(Creature* pCreature)
{
    return new deathside_mob_shackles_of_darknessAI(pCreature);
}

CreatureAI* GetAI_deathside_boss_soul_blighterAI(Creature* pCreature)
{
return new deathside_boss_soul_blighterAI (pCreature);
}

void AddSC_deathside_boss_soul_blighter()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "deathside_mob_shackles_of_darkness";
    newscript->GetAI = &GetAI_deathside_mob_shackles_of_darkness;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_boss_soul_blighter";
    newscript->GetAI = &GetAI_deathside_boss_soul_blighterAI;
    newscript->RegisterSelf();

    
}
