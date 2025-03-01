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
SDName: Illidari_Council
SD%Complete: 99
SDComment: Test event reseting and saving to DB (simplify some code..?)
SDCategory: Black Temple
EndScriptData */

//TODO: Integrate all 4 EnterEvadeModes into illidari_council_baseAI::EnterEvadeMode().

#include "precompiled.h"
#include "def_black_temple.h"

//Speech'n'Sounds
#define SAY_GATH_SLAY           -1564085
#define SAY_GATH_SLAY_COMNT     -1564089
#define SAY_GATH_DEATH          -1564093
#define SAY_GATH_SPECIAL1       -1564077
#define SAY_GATH_SPECIAL2       -1564081

#define SAY_VERA_SLAY           -1564086
#define SAY_VERA_COMNT          -1564089
#define SAY_VERA_DEATH          -1564094
#define SAY_VERA_SPECIAL1       -1564078
#define SAY_VERA_SPECIAL2       -1564082

#define SAY_MALA_SLAY           -1564087
#define SAY_MALA_COMNT          -1564090
#define SAY_MALA_DEATH          -1564095
#define SAY_MALA_SPECIAL1       -1564079
#define SAY_MALA_SPECIAL2       -1564083

#define SAY_ZERE_SLAY           -1564088
#define SAY_ZERE_COMNT          -1564091
#define SAY_ZERE_DEATH          -1564096
#define SAY_ZERE_SPECIAL1       -1564080
#define SAY_ZERE_SPECIAL2       -1564084

struct CouncilYells
{
    int32 entry;
    uint32 timer;
};

static CouncilYells CouncilAggro[]=
{
    {-1564069, 5000},                                       // Gathios
    {-1564070, 5500},                                       // Veras
    {-1564071, 5000},                                       // Malande
    {-1564072, 1200000},                                    // Zerevor
};

// Need to get proper timers for this later
static CouncilYells CouncilEnrage[]=
{
    {-1564073, 2000},                                       // Gathios
    {-1564074, 6000},                                       // Veras
    {-1564075, 5000},                                       // Malande
    {-1564076, 1200000},                                    // Zerevor
};

#define SPELL_BERSERK 45078

enum councilMembers
{
    NPC_GATHIOS     = 22949,
    NPC_ZEREVOR     = 22950,
    NPC_MALANDE     = 22951,
    NPC_VERAS       = 22952
};

// Gathios the Shatterer's spells
enum gathiosSpells
{
    SPELL_BLESS_PROTECTION     = 41450,
    SPELL_BLESS_SPELLWARD      = 41451,
    SPELL_CONSECRATION         = 41541,
    SPELL_HAMMER_OF_JUSTICE    = 41468,
    SPELL_SEAL_OF_COMMAND      = 41469,
    SPELL_SEAL_OF_BLOOD        = 41459,
    SPELL_GATHIOS_JUDGEMENT    = 41467,
    SPELL_CHROMATIC_AURA       = 41453,
    SPELL_DEVOTION_AURA        = 41452
};

// High Nethermancer Zerevor's spells
enum zerevorSpells
{
    SPELL_FLAMESTRIKE       = 41481,
    SPELL_BLIZZARD          = 41482,
    SPELL_ARCANE_BOLT       = 41483,
    SPELL_ARCANE_EXPLOSION  = 41524,
    SPELL_DAMPEN_MAGIC      = 41478
};

// Lady Malande's spells
enum malandeSpells
{
    SPELL_EMPOWERED_SMITE   = 41471,
    SPELL_CIRCLE_OF_HEALING = 41455,
    SPELL_REFLECTIVE_SHIELD = 41475,
    SPELL_DIVINE_WRATH      = 41472,
    SPELL_HEAL_VISUAL       = 24171
};

// Veras Darkshadow's spells
enum verasSpells
{
    SPELL_DEADLY_POISON     = 41480,
    SPELL_VANISH            = 41476,
    SPELL_SNEAK             = 8218, // surely not this one but works
    // Spell Envenom triggered by Deadly Poison in Aura::HandlePeriodicDamage
};

struct mob_blood_elf_council_voice_triggerAI : public ScriptedAI
{
    mob_blood_elf_council_voice_triggerAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)me->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint64 m_council[4];

    Timer_UnCheked m_enrageTimer;
    Timer_UnCheked m_yellTimer;

    uint8 m_counter;                                      // Serves as the counter for both the aggro and enrage yells

    void Reset()
    {
        m_enrageTimer.Reset(900000);                               // 15 minutes
        m_yellTimer.Reset(500);

        m_counter = 0;
    }

    void AssignGUIDs()
    {
        m_council[0] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
        m_council[1] = pInstance->GetData64(DATA_VERASDARKSHADOW);
        m_council[2] = pInstance->GetData64(DATA_LADYMALANDE);
        m_council[3] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
    }

    void AttackStart(Unit *pWho){}

    void MoveInLineOfSight(Unit *pWho){}

    void UpdateAI(const uint32 diff)
    {
        if (pInstance->GetData(EVENT_ILLIDARICOUNCIL) != IN_PROGRESS)
            return;

        if (m_counter > 3)
            return;

        if (m_yellTimer.Expired(diff))
        {
            if (Unit *pMember = me->GetCreature(m_council[m_counter]))
            {
                DoScriptText(CouncilAggro[m_counter].entry, pMember);
                m_yellTimer = CouncilAggro[m_counter].timer;
            }

            m_counter += 1;
            if (m_counter > 3)
                m_counter = 0;                            // Reuse for Enrage Yells
        }
        

        if (m_enrageTimer.Expired(diff))
        {
            if (Creature* pMember = pInstance->GetCreature(m_council[m_counter]))
            {
                pMember->CastSpell(pMember, SPELL_BERSERK, true);

                DoScriptText(CouncilEnrage[m_counter].entry, pMember);
                m_enrageTimer = CouncilEnrage[m_counter].timer;
            }

            m_counter += 1;
        }
    }
};

struct mob_illidari_councilAI : public ScriptedAI
{
    mob_illidari_councilAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    uint64 m_council[4];

    void Reset()
    {
        pInstance->SetData(EVENT_ILLIDARICOUNCIL, NOT_STARTED);

        if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->AI()->EnterEvadeMode();

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetDisplayId(11686);
    }

    void StartEvent(Unit *pTarget)
    {
        if (pTarget->isAlive())
        {
            m_council[0] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
            m_council[1] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
            m_council[2] = pInstance->GetData64(DATA_LADYMALANDE);
            m_council[3] = pInstance->GetData64(DATA_VERASDARKSHADOW);

            // Start the event for the Voice Trigger
            if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
                ((mob_blood_elf_council_voice_triggerAI*)pTrigger->AI())->AssignGUIDs();

            for (uint8 i = 0; i < 4; ++i)
            {
                if (m_council[i])
                {
                    if (Unit *pMember = pInstance->GetCreature(m_council[i]))
                    {
                        if (pMember->isAlive())
                            ((Creature*)pMember)->AI()->AttackStart(pTarget);
                    }
                }
            }
            pInstance->SetData(EVENT_ILLIDARICOUNCIL, IN_PROGRESS);
        }
    }

    void JustDied(Unit *Killer)
    {
        if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->Kill(pTrigger, false);

        pInstance->SetData(EVENT_ILLIDARICOUNCIL, DONE);
        if (Creature *pAkama = me->SummonCreature(23089, 671.309f, 305.427f, 271.689f, 6.068f, TEMPSUMMON_DEAD_DESPAWN, 0))
            pAkama->AI()->DoAction(6);
    }
};

struct illidari_council_baseAI : public ScriptedAI
{
    illidari_council_baseAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        loadedGUIDs = false;
    }

    uint64 m_council[4];

    Timer_UnCheked m_checkTimer;

    ScriptedInstance* pInstance;

    bool loadedGUIDs;

    void EnterCombat(Unit *pWho)
    {
        if (Creature *pController = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDARICOUNCIL)))
            ((mob_illidari_councilAI*)pController->AI())->StartEvent(pWho);

        DoZoneInCombat();

        if (me->GetEntry() == NPC_ZEREVOR)  // Zerevor
        {
            ClearCastQueue();
            ForceSpellCast(me, SPELL_DAMPEN_MAGIC);
            AddSpellToCast(pWho, SPELL_ARCANE_BOLT);
            StartAutocast();
        }

        if (!loadedGUIDs)
        {
            m_council[0] = pInstance->GetData64(DATA_LADYMALANDE);
            m_council[1] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
            m_council[2] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
            m_council[3] = pInstance->GetData64(DATA_VERASDARKSHADOW);
            loadedGUIDs = true;
        }
    }

    void KilledUnit(Unit *pVictim)
    {
        switch (me->GetEntry())
        {
            case NPC_GATHIOS: DoScriptText(SAY_GATH_SLAY, me); break; // Gathios
            case NPC_ZEREVOR: DoScriptText(SAY_ZERE_SLAY, me); break; // Zerevor
            case NPC_MALANDE: DoScriptText(SAY_MALA_SLAY, me); break; // Melande
            case NPC_VERAS: DoScriptText(SAY_VERA_SLAY, me); break; // Veras
        }
    }

    void JustDied(Unit *pVictim)
    {
        switch (me->GetEntry())
        {
            case NPC_GATHIOS: DoScriptText(SAY_GATH_DEATH, me); break; // Gathios
            case NPC_ZEREVOR: DoScriptText(SAY_ZERE_DEATH, me); break; // Zerevor
            case NPC_MALANDE: DoScriptText(SAY_MALA_DEATH, me); break; // Melande
            case NPC_VERAS: DoScriptText(SAY_VERA_DEATH, me); break; // Veras
        }

        if (Creature *priest = GetClosestCreatureWithEntry(me, NPC_MALANDE, 100.0f, true))
            if (priest->isAlive())
                priest->Kill(priest);

        if (Creature *rogue = GetClosestCreatureWithEntry(me, NPC_VERAS, 100.0f, true))
            if (rogue->isAlive())
                rogue->Kill(rogue);

        if (Creature *mage = GetClosestCreatureWithEntry(me, NPC_ZEREVOR, 100.0f, true))
            if (mage->isAlive())
                mage->Kill(mage);

        if (Creature *paladin = GetClosestCreatureWithEntry(me, NPC_GATHIOS, 100.0f, true))
            if (paladin->isAlive())
                paladin->Kill(paladin);


        if (Creature *pCouncil = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDARICOUNCIL)))
        {
            if (pCouncil->isAlive())
                pCouncil->Kill(pCouncil, false);
        }
    }

    void EnterEvadeMode()
    {
        if (!pInstance)
            return;
        pInstance->SetData(EVENT_ILLIDARICOUNCIL, NOT_STARTED);

        ScriptedAI::EnterEvadeMode();

        if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->AI()->EnterEvadeMode();

            if (Creature *mage = GetClosestCreatureWithEntry(me, NPC_ZEREVOR, 100.0f, true))
                if (mage->IsInCombat())
                    mage->AI()->EnterEvadeMode();

            if (Creature *rogue = GetClosestCreatureWithEntry(me, NPC_VERAS, 100.0f, true))
                if (rogue->IsInCombat())
                    rogue->AI()->EnterEvadeMode();

            if (Creature *priest = GetClosestCreatureWithEntry(me, NPC_MALANDE, 100.0f, true))
                if (priest->IsInCombat())
                    priest->AI()->EnterEvadeMode();

            if (Creature *paladin = GetClosestCreatureWithEntry(me, NPC_GATHIOS, 100.0f, true))
                if (paladin->IsInCombat())
                    paladin->AI()->EnterEvadeMode();




    }

    void SharedRule(uint32 &damage)
    {
        uint32 HP = 0;
        bool canKill = false;
        // get shared HP
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if (pUnit->GetHealth() == 0)
                    canKill = true;
                HP += pUnit->GetHealth();
                pUnit->LowerPlayerDamageReq(damage);
            }
        }
        // set shared HP
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if (pUnit->isAlive())
                {
                    if (HP)
                        pUnit->SetHealth(HP/4);
                }
            }
        }
        // if one dies, they die all
        if (!canKill)
            return;
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if (pUnit->isAlive())
                    pUnit->Kill(pUnit, false);
            }
        }
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (done_by == me)
            return;
        SharedRule(damage);
    }
};

// Gathios the Shatterer's AI
struct boss_gathios_the_shattererAI : public illidari_council_baseAI
{
    boss_gathios_the_shattererAI(Creature *c) : illidari_council_baseAI(c)
    {
        c->GetPosition(wLoc);
    }

    WorldLocation wLoc;

    Timer_UnCheked m_sealTimer;
    Timer_UnCheked m_auraTimer;
    Timer_UnCheked m_hammerTimer;
    Timer_UnCheked m_blessingTimer;
    Timer_UnCheked m_judgementTimer;
    Timer_UnCheked m_consecrationTimer;

    Timer_UnCheked m_checkTimer;

    void Reset()
    {
        ClearCastQueue();

        m_consecrationTimer.Reset(urand(6000, 10000));
        m_hammerTimer.Reset(10000);
        m_sealTimer.Reset(1000);
        m_auraTimer.Reset(urand(3000, 30000));
        m_blessingTimer.Reset(urand(35000, 50000));
        m_judgementTimer.Reset(16000);

        m_checkTimer.Reset(1000);
    }

    Unit* SelectCouncil()
    {
        if (urand(0, 8)) // 8/9 chances to select Malande
        {
            if (Unit *pMelande = pInstance->GetCreature(m_council[0]))
            {
                if (pMelande->isAlive())
                    return pMelande;
            }
        }

        if (Unit *pCouncil = pInstance->GetCreature(m_council[urand(0,1)?1:3])) // else, select others, but never self
        {
            if (pCouncil->isAlive())
                return pCouncil;
        }

        return me;
    }

    void RegenMana()
    {
        uint32 maxMana = me->GetMaxPower(POWER_MANA);
        uint32 Mana = me->GetPower(POWER_MANA);
        me->SetPower(POWER_MANA, Mana+0.01*maxMana);
    }



    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer.Expired(diff))
        {
            if (me->IsWithinDistInMap(&wLoc, 100.0f))
                DoZoneInCombat();
            else
            {
                EnterEvadeMode();
                return;
            }

            // me->SetSpeed(MOVE_RUN, 2.0);
            uint32 damage = 0;
            SharedRule(damage);
            m_checkTimer = 1000;
        }
        

        if (m_blessingTimer.Expired(diff))
            if (Unit *pUnit = SelectCouncil())
            {
                AddSpellToCast(pUnit, RAND(SPELL_BLESS_SPELLWARD, SPELL_BLESS_PROTECTION));
                m_blessingTimer = RAND(urand(15000, 20000), urand(25000, 35000));
            }
        
        

        if (m_consecrationTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CONSECRATION);
            m_consecrationTimer = urand(30000, 35000);
        }
        

        if (m_hammerTimer.Expired(diff))
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true, 0, 10.0f))
            {
                AddSpellToCast(pTarget, SPELL_HAMMER_OF_JUSTICE);
                m_hammerTimer = urand(10000, 20000);
            }
        
        

        if (m_sealTimer.Expired(diff))
        {
            AddSpellToCast(me, RAND(SPELL_SEAL_OF_COMMAND, SPELL_SEAL_OF_BLOOD));
            m_sealTimer = urand(17000, 20000);
        }
        

        if (m_judgementTimer.Expired(diff))
        {
            RegenMana();
            ForceSpellCast(me->GetVictim(), SPELL_GATHIOS_JUDGEMENT, INTERRUPT_AND_CAST);
            m_judgementTimer = 15000;
        }
        

        if (m_auraTimer.Expired(diff))
        {
            AddSpellToCast(RAND(SPELL_DEVOTION_AURA, SPELL_CHROMATIC_AURA), CAST_SELF);
            m_auraTimer = 60000;
        }
        

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

// High Nethermancer Zerevor's AI
struct boss_high_nethermancer_zerevorAI : public illidari_council_baseAI
{
    boss_high_nethermancer_zerevorAI(Creature *c) : illidari_council_baseAI(c)
    {
        c->GetPosition(wLoc);
    }

    WorldLocation wLoc;

    Timer_UnCheked m_blizzardTimer;
    Timer_UnCheked m_flamestrikeTimer;
    Timer_UnCheked m_dampenTimer;
    Timer_UnCheked m_aexpTimer;
    Timer_UnCheked m_immunityTimer;

    void Reset()
    {
        ClearCastQueue();

        m_blizzardTimer.Reset(urand(12000, 20000));
        m_flamestrikeTimer.Reset(3800);
        m_dampenTimer.Reset(67200);
        m_aexpTimer.Reset(3000);
        m_immunityTimer.Reset(0);
        SetAutocast(SPELL_ARCANE_BOLT, 2000, true, CAST_TANK, 40.0f, true);

        m_checkTimer = 1000;
    }

    void CheckStairsPos()   // nasty workaround, remove when other options available
    {
        float x, y, z, good_z;
        float top_x = 688.7;
        float top_z = 277.5;
        x = me->GetPositionX();
        y = me->GetPositionY();
        z = me->GetPositionZ();
        if (y > 286 && y < 324 && x > 676.2)
        {
            if (top_x > x)
                good_z = 0.452*(top_x-x);
            else
                good_z = top_z;

            if (z < good_z)
                me->GetMap()->CreatureRelocation(me, x, y, good_z+0.15, me->GetOrientationTo(me->GetVictim()));
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer.Expired(diff))
        {
            if (me->IsWithinDistInMap(&wLoc, 100.0f))
                DoZoneInCombat();
            else
            {
                EnterEvadeMode();
                return;
            }

            if (me->GetDistance(me->GetVictim()) <= 40.0f)
            {
                me->SetFacingToObject(me->GetVictim());  // he is getting stuck sometimes
                me->StopMoving();

            }
            else
                DoStartMovement(me->GetVictim());
                //me->GetMotionMaster()->MoveChase(me->GetVictim(), 40.0f);  // It's not working. Simply.

            // On front stairs, do not let boss to go into textures;
            CheckStairsPos();

            uint32 damage = 0;
            SharedRule(damage);
            // me->SetSpeed(MOVE_RUN, 2.0);
            m_checkTimer = 1000;
        }
        

        if (m_immunityTimer.Expired(diff))
        {
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
            m_immunityTimer = 0;
        }
        

        if (m_dampenTimer.Expired(diff))
        {
            ForceSpellCast(me, SPELL_DAMPEN_MAGIC, DONT_INTERRUPT, true, true);
            m_dampenTimer = 67200;                      // almost 1,12 minutes (??)
        }
        

        if (m_blizzardTimer.Expired(diff))
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                AddSpellToCast(pTarget, SPELL_BLIZZARD, true);
                m_blizzardTimer = urand(11000, 17000);
            }
        
        

        if (m_flamestrikeTimer.Expired(diff))
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                ForceSpellCast(pTarget, SPELL_FLAMESTRIKE);
                m_flamestrikeTimer = urand(15000, 20000);
                m_immunityTimer.Reset(3000);
            }
        }

        if (m_aexpTimer.Expired(diff))
        {
            std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::iterator i = m_threatlist.begin(); i!= m_threatlist.end();++i)
            {
                if (Unit* pUnit = Unit::GetUnit((*me), (*i)->getUnitGuid()))
                {
                    if (pUnit->IsWithinDistInMap(me, 5) && pUnit->GetTypeId() == TYPEID_PLAYER && pUnit->isAlive() && !pUnit->IsImmunedToDamage(SPELL_SCHOOL_MASK_ARCANE))
                    {
                        ForceSpellCast(SPELL_ARCANE_EXPLOSION, CAST_SELF);
                        m_aexpTimer = 3000;
                        break;
                    }
                    else
                        m_aexpTimer = 1000;
                }
            }
        }


        CastNextSpellIfAnyAndReady(diff);
    }
};

// Lady Malande's AI
struct boss_lady_malandeAI : public illidari_council_baseAI
{
    boss_lady_malandeAI(Creature *c) : illidari_council_baseAI(c)
    {
        c->GetPosition(wLoc);
    }

    WorldLocation wLoc;

    Timer_UnCheked m_smiteTimer;
    Timer_UnCheked m_cohTimer;
    Timer_UnCheked m_wrathTimer;
    Timer_UnCheked m_shieldTimer;

    void Reset()
    {
        ClearCastQueue();

        m_smiteTimer.Reset(200);
        m_cohTimer.Reset(20000);
        m_wrathTimer.Reset(urand(8000, 12000));
        m_shieldTimer.Reset(15000);

        m_checkTimer.Reset(1000);
    }





    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer.Expired(diff))
        {
            if (me->IsWithinDistInMap(&wLoc, 100.0f))
                DoZoneInCombat();
            else
            {
                EnterEvadeMode();
                return;
            }

            uint32 damage = 0;
            SharedRule(damage);
            // me->SetSpeed(MOVE_RUN, 2.0);
            m_checkTimer = 1000;
        }
        
        if (m_smiteTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_EMPOWERED_SMITE, false, true);
            m_smiteTimer = urand(5000, 9000);
        }
        

        if (m_cohTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CIRCLE_OF_HEALING);
            m_cohTimer = urand(19000, 23000);
        }
        

        if (m_wrathTimer.Expired(diff))
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                AddSpellToCast(pTarget, SPELL_DIVINE_WRATH, false, true);
                m_wrathTimer = urand(15000, 30000);
            }
        
        

        if (m_shieldTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_REFLECTIVE_SHIELD);
            m_shieldTimer = urand(40000, 55000);
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

// Veras Darkshadow's AI
struct boss_veras_darkshadowAI : public illidari_council_baseAI
{
    boss_veras_darkshadowAI(Creature *c) : illidari_council_baseAI(c)
    {
        c->GetPosition(wLoc);
    }

    WorldLocation wLoc;

    Timer m_vanishTimer;
    Timer m_sneakTimer;
    Timer AttackTimer;
    bool CanAttack;

    void Reset()
    {
        ClearCastQueue();

        m_vanishTimer.Reset(urand(15000, 25000));
        m_checkTimer.Reset(1000);
        m_sneakTimer.Reset(0);
        AttackTimer.Reset(0);
        me->RemoveAurasDueToSpell(SPELL_SNEAK);
        CanAttack = true;
    }

    void OnAuraRemove(Aura* aur, bool stackRemove)
    {
        if (aur->GetId() == SPELL_VANISH)
        {
            ForceSpellCast(me, SPELL_SNEAK, INTERRUPT_AND_CAST_INSTANTLY, true);
            m_sneakTimer.Reset(4000);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, false);
            DoResetThreat();
        }
        if (aur->GetId() == SPELL_SNEAK)
        {
            me->SetIgnoreVictimSelection(false);
            AttackTimer.Reset(1300);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_sneakTimer.Expired(diff))
        {
            m_sneakTimer = 0;
            me->RemoveAurasDueToSpell(SPELL_SNEAK);
            DoStartMovement(me->GetVictim());
            me->SetSpeed(MOVE_RUN, 1.3);
        }

        if(AttackTimer.Expired(diff))
        {
            AttackTimer = 0;
            CanAttack = true;
        }

        if (m_checkTimer.Expired(diff))
        {
            if (me->IsWithinDistInMap(&wLoc, 100.0f))
                DoZoneInCombat();
            else
            {
                EnterEvadeMode();
                return;
            }

            uint32 damage = 0;
            SharedRule(damage);
            // 
            // move always after stun recovery
            if (!me->HasUnitState(UNIT_STAT_STUNNED) && !me->HasAura(SPELL_VANISH, 1) && !me->HasAura(SPELL_SNEAK, 0))
                DoStartMovement(me->GetVictim());
            m_checkTimer = 1000;
        }
        

        if (m_vanishTimer.Expired(diff))
        {
            CanAttack = false;
            me->SetSpeed(MOVE_RUN, 2.0);
            if (me->HasAuraType(SPELL_AURA_MOD_STUN))    // remove stun
                me->RemoveSpellsCausingAura(SPELL_AURA_MOD_STUN);

            if (me->HasAuraType(SPELL_AURA_MOD_STALKED)) // remove Hunter's Marks and similar trackers
                me->RemoveSpellsCausingAura(SPELL_AURA_MOD_STALKED);

            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);

            ForceSpellCast(me, SPELL_VANISH, INTERRUPT_AND_CAST_INSTANTLY);
            ForceSpellCast(me, SPELL_DEADLY_POISON, INTERRUPT_AND_CAST_INSTANTLY);

            Position dest;
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 35.0f, true))
                target->GetValidPointInAngle(dest, 5.0f, frand(0.0f, 2*M_PI), true);
            else
                me->GetValidPointInAngle(dest, 30.0f, frand(0.0f, 2*M_PI), true);

            DoTeleportTo(dest.x, dest.y, dest.z);

            // drop movement :P
            me->GetMotionMaster()->MoveIdle();
            me->SetIgnoreVictimSelection(true);
            m_vanishTimer = 60000;
        }
        

        if (me->HasAura(SPELL_VANISH) || me->HasAura(SPELL_SNEAK) || !CanAttack)
            return;

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_mob_blood_elf_council_voice_trigger(Creature* c)
{
    return new mob_blood_elf_council_voice_triggerAI(c);
}

CreatureAI* GetAI_mob_illidari_council(Creature *_Creature)
{
    return new mob_illidari_councilAI (_Creature);
}

CreatureAI* GetAI_boss_gathios_the_shatterer(Creature *_Creature)
{
    return new boss_gathios_the_shattererAI (_Creature);
}

CreatureAI* GetAI_boss_lady_malande(Creature *_Creature)
{
    return new boss_lady_malandeAI (_Creature);
}

CreatureAI* GetAI_boss_veras_darkshadow(Creature *_Creature)
{
    return new boss_veras_darkshadowAI (_Creature);
}

CreatureAI* GetAI_boss_high_nethermancer_zerevor(Creature *_Creature)
{
    return new boss_high_nethermancer_zerevorAI (_Creature);
}

void AddSC_boss_illidari_council()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_illidari_council";
    newscript->GetAI = &GetAI_mob_illidari_council;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_blood_elf_council_voice_trigger";
    newscript->GetAI = &GetAI_mob_blood_elf_council_voice_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_gathios_the_shatterer";
    newscript->GetAI = &GetAI_boss_gathios_the_shatterer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_lady_malande";
    newscript->GetAI = &GetAI_boss_lady_malande;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_veras_darkshadow";
    newscript->GetAI = &GetAI_boss_veras_darkshadow;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_high_nethermancer_zerevor";
    newscript->GetAI = &GetAI_boss_high_nethermancer_zerevor;
    newscript->RegisterSelf();
}
