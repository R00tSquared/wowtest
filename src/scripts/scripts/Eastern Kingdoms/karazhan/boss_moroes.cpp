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
SDName: Boss_Moroes
SD%Complete: 95
SDComment:
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

enum Moroes
{
    SAY_AGGRO           = -1532011,
    SAY_SPECIAL_1       = -1532012,
    SAY_SPECIAL_2       = -1532013,
    SAY_KILL_1          = -1532014,
    SAY_KILL_2          = -1532015,
    SAY_KILL_3          = -1532016,
    SAY_DEATH           = -1532017,
    SAY_RP_1            = -1811011,
    SAY_RP_2            = -1811012,
    SAY_RP_3            = -1811013,
    SAY_RP_4            = -1811014,
    SAY_RP_1_1          = -1811015,
    SAY_RP_1_2          = -1811016,
    SAY_RP_1_3          = -1811017,
    SAY_RP_1_4          = -1811018,
    SAY_RP_1_5          = -1811019,
    SPELL_VANISH        = 29448,
    SPELL_GARROTE       = 37066,
    SPELL_BLIND         = 34694,
    SPELL_GOUGE         = 29425,
    SPELL_FRENZY        = 37023,
    SPELL_DEADLY_THROW  = 37074 // ??
};

#define POS_Z               81.73

float Locations[4][3]=
{
    {-10991.0, -1884.33, 0.614315},
    {-10989.4, -1885.88, 0.904913},
    {-10978.1, -1887.07, 2.035550},
    {-10975.9, -1885.81, 2.253890},
};

const uint32 Adds[6]=
{
    17007,
    19872,
    19873,
    19874,
    19875,
    19876,
};

struct boss_moroesAI : public ScriptedAI
{
    boss_moroesAI(Creature *c) : ScriptedAI(c)
    {
        for(int i = 0; i < 4; i++)
        {
            AddId[i] = 0;
        }
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    uint64 AddGUID[4];

    Timer Vanish_Timer;
    Timer Blind_Timer;
    Timer Gouge_Timer;
    Timer Garrote_Timer;
    Timer NonAttackable_Timer;
    Timer CheckAdds_Timer;
    Timer DeadlyThrow_Timer;
    Timer RPSay_Timer;
    
    uint32 AddId[4];
    uint8 RP_Phase;

    bool InVanish;
    bool NonAttackable;
    bool Enrage;

    void Reset()
    {
        Vanish_Timer.Reset(urand(35000, 40000));
        Blind_Timer.Reset(urand(30000, 35000));
        Gouge_Timer.Reset(urand(25000, 30000));
        DeadlyThrow_Timer.Reset(10 * MINUTE * MILLISECONDS);
        CheckAdds_Timer.Reset(5000);
        RPSay_Timer.Reset(urand(10000, 15000));
        Garrote_Timer.Reset(0);
        NonAttackable_Timer.Reset(0);

        RP_Phase = 0;

        Enrage = false;
        InVanish = false;
        NonAttackable = false;

        if(me->GetHealth() > 0)
            SpawnAdds();

        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

        if(pInstance && pInstance->GetData(DATA_MOROES_EVENT) != DONE)
            pInstance->SetData(DATA_MOROES_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, me);
        AddsAttack();
        DoZoneInCombat();
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2, SAY_KILL_3), me);
    }

    void JustDied(Unit* victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, DONE);

        // remove aura from spell Garrote when Moroes dies
        Map *map = me->GetMap();
        if (map->IsDungeon())
        {
            Map::PlayerList const &PlayerList = map->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (i->getSource()->isAlive() && i->getSource()->HasAura(SPELL_GARROTE, 0))
                    i->getSource()->RemoveAurasDueToSpell(SPELL_GARROTE);
            }
        }
    }

    void SpawnAdds()
    {
        DeSpawnAdds();

        if(isAddlistEmpty())
        {
            Creature *pCreature = NULL;
            std::vector<uint32> AddList;


            for(uint8 i = 0; i < 6; ++i)
                AddList.push_back(Adds[i]);

            while(AddList.size() > 4)
                AddList.erase((AddList.begin())+(rand()%AddList.size()));

            uint8 i = 0;
            for(std::vector<uint32>::iterator itr = AddList.begin(); itr != AddList.end(); ++itr)
            {
                uint32 entry = *itr;

                pCreature = me->SummonCreature(entry, Locations[i][0], Locations[i][1], POS_Z, Locations[i][2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                if (pCreature)
                {
                    AddGUID[i] = pCreature->GetGUID();
                    AddId[i] = entry;
                }
                ++i;
            }
        }
        else
        {
            for(int i = 0; i < 4; i++)
            {
                Creature *pCreature = me->SummonCreature(AddId[i], Locations[i][0], Locations[i][1], POS_Z, Locations[i][2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                if (pCreature)
                    AddGUID[i] = pCreature->GetGUID();
            }
        }
    }

    bool isAddlistEmpty()
    {
        for(int i = 0; i < 4; i++)
        {
            if(AddId[i] == 0)
                return true;
        }
        return false;
    }

    void DeSpawnAdds()
    {
        for(uint8 i = 0; i < 4 ; ++i)
        {
            Creature* Temp = NULL;
            if (AddGUID[i])
            {
                Temp = Creature::GetCreature((*me),AddGUID[i]);
                if (Temp && Temp->isAlive())
                {
                    (*Temp).GetMotionMaster()->Clear(true);
                    Temp->DealDamage(Temp, Temp->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    Temp->RemoveCorpse();
                }

            }
        }
    }

    void AddsAttack()
    {
        for(uint8 i = 0; i < 4; ++i)
        {
            Creature* Temp = NULL;
            if (AddGUID[i])
            {
                Temp = Creature::GetCreature((*me),AddGUID[i]);
                if (Temp && Temp->isAlive())
                {
                    Temp->AI()->AttackStart(me->GetVictim());
                    Temp->AI()->DoZoneInCombat();
                }
                else
                    EnterEvadeMode();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(RPSay_Timer.Expired(diff))
            {
                if(RP_Phase == 0)
                {
                    std::vector<Creature*> AddList;
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        Creature* Temp = NULL;
                        if (AddGUID[i])
                        {
                            Temp = Unit::GetCreature((*me),AddGUID[i]);
                            if (Temp && Temp->isAlive())
                                AddList.push_back(Temp);
                        }
                    }
                    DoScriptText(RAND(SAY_RP_1, SAY_RP_2, SAY_RP_3, SAY_RP_4), !AddList.empty() ? AddList[urand(0, AddList.size() -1)] : me, 0);
                    RPSay_Timer = 4000;
                    RP_Phase = 1;
                }
                else if(RP_Phase == 1)
                {
                    DoScriptText(RAND(SAY_RP_1_1, SAY_RP_1_2, SAY_RP_1_3, SAY_RP_1_4, SAY_RP_1_5), me, 0);
                    RPSay_Timer = urand (15000, 25000);
                    RP_Phase = 0;
                }
            }
            return;
        }

        if(pInstance && !pInstance->GetData(DATA_MOROES_EVENT))
        {
            EnterEvadeMode();
            return;
        }

        if(!Enrage && me->GetHealthPercent() < 30)
        {
            DoCast(me, SPELL_FRENZY);
            Enrage = true;
        }

        if (CheckAdds_Timer.Expired(diff))
        {
            for (uint8 i = 0; i < 4; ++i)
            {
                Creature* Temp = NULL;
                if (AddGUID[i])
                {
                    Temp = Unit::GetCreature((*me),AddGUID[i]);
                    if (Temp && Temp->isAlive())
                        if (!Temp->GetVictim())
                            Temp->AI()->AttackStart(me->GetVictim());
                }
            }
            CheckAdds_Timer = 5000;
        }

        if(DeadlyThrow_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_DEADLY_THROW);
            // todo :add emote!
            DeadlyThrow_Timer = 5000;
        }

        if (!InVanish)
        {
            //Cast Vanish, then Garrote random victim
            if (Vanish_Timer.Expired(diff))
            {
                DoCast(me, SPELL_VANISH);
                InVanish = true;
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                NonAttackable = true;
                Vanish_Timer = urand(35000, 40000);
                NonAttackable_Timer = Garrote_Timer = urand(3000, 8000);
            }

            if (Gouge_Timer.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_GOUGE);
                DoModifyThreatPercent(me->GetVictim(), -10);
                DoModifyThreatPercent(SelectUnit(SELECT_TARGET_TOPAGGRO, 1), 100);
                Gouge_Timer = urand(25000, 30000);
            }

            if (Blind_Timer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, GetSpellMaxRange(SPELL_BLIND), true))
                    DoCast(target, SPELL_BLIND);

                Blind_Timer = urand(30000, 35000);
            }
        }

        if(InVanish)
        {
            if (Garrote_Timer.Expired(diff))
            {
                std::vector<Unit*> ungarrotedPlayers;
                std::list<HostileReference*>& threatList = me->getThreatManager().getThreatList();
                for(std::list<HostileReference*>::iterator i = threatList.begin(); i != threatList.end(); ++i)
                {
                    if (Unit* target = Unit::GetUnit((*me), (*i)->getUnitGuid()))
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER && !target->HasAura(SPELL_GARROTE))
                            ungarrotedPlayers.push_back(target);
                    }
                }

                Unit* target = !ungarrotedPlayers.empty() ? ungarrotedPlayers[urand(0, ungarrotedPlayers.size() -1)] : SelectUnit(SELECT_TARGET_RANDOM, 0, 50, true);
                if(target)
                {
                    me->NearTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation());
                    DoScriptText(RAND(SAY_SPECIAL_1, SAY_SPECIAL_2), me);
                    target->CastSpell(target, SPELL_GARROTE, true, NULL, NULL, me->GetGUID());
                    me->RemoveAurasDueToSpell(SPELL_VANISH);
                }

                InVanish = false;
                Garrote_Timer = 0;
            }
        }

        if(!InVanish)
            DoMeleeAttackIfReady();

        if(NonAttackable)
        {
            if (NonAttackable_Timer.Expired(diff))
            {
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                NonAttackable = false;
                NonAttackable_Timer = 0;
            } 
        }
    }
};

struct boss_moroes_guestAI : public ScriptedAI
{
    ScriptedInstance* pInstance;

    uint64 GuestGUID[4];

    boss_moroes_guestAI(Creature* c) : ScriptedAI(c)
    {
        for(uint8 i = 0; i < 4; ++i)
            GuestGUID[i] = 0;

        pInstance = (c->GetInstanceData());
    }

    Timer CheckRange_Timer;

    void Reset()
    {
        if(pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, NOT_STARTED);
        CheckRange_Timer = 5000;
    }

    void AcquireGUID()
    {
        if(!pInstance)
            return;

        GuestGUID[0] = pInstance->GetData64(DATA_MOROES);
        Creature* Moroes = (Unit::GetCreature((*me), GuestGUID[0]));
        if(Moroes)
        {
            for(uint8 i = 0; i < 3; ++i)
            {
                uint64 GUID = ((boss_moroesAI*)Moroes->AI())->AddGUID[i];
                if(GUID && GUID != me->GetGUID())
                    GuestGUID[i+1] = GUID;
            }
        }
    }

    Unit* SelectTarget()
    {
        uint64 TempGUID = GuestGUID[rand()%5];
        if(TempGUID)
        {
            Unit* pUnit = Unit::GetUnit((*me), TempGUID);
            if(pUnit && pUnit->isAlive())
                return pUnit;
        }
        return me;
    }

    void UpdateAI(const uint32 diff)
    {
        if(pInstance && !pInstance->GetData(DATA_MOROES_EVENT))
            EnterEvadeMode();

        if(CheckRange_Timer.Expired(diff))
        {
            if(me->GetDistance(-10983.29, -1881.109, 81.728) > 70.0f)
            {
                Creature* Moroes = (Unit::GetCreature((*me), pInstance->GetData64(DATA_MOROES)));
                if(Moroes && Moroes->isAlive())
                    Moroes->AI()->EnterEvadeMode();
                else
                    me->DisappearAndDie();
            }
            CheckRange_Timer = 3000;
        }

        DoMeleeAttackIfReady();
    }
};

#define SPELL_MANABURN       29405
#define SPELL_MINDFLY        29570
#define SPELL_SWPAIN         34441
#define SPELL_SHADOWFORM     29406

struct boss_baroness_dorothea_millstipeAI : public boss_moroes_guestAI
{
    //Shadow Priest
    boss_baroness_dorothea_millstipeAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer ManaBurn_Timer;
    Timer MindFlay_Timer;
    Timer ShadowWordPain_Timer;

    void Reset()
    {
        ManaBurn_Timer.Reset(7000);
        MindFlay_Timer.Reset(1000);
        ShadowWordPain_Timer.Reset(6000);
        if(me->HasAura(SPELL_SHADOWFORM))
            me->RemoveAurasDueToSpell(SPELL_SHADOWFORM);

        boss_moroes_guestAI::Reset();
    }

    void EnterCombat(Unit * pWho)
    {
        DoCast(me, SPELL_SHADOWFORM, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if (MindFlay_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_MINDFLY);
            MindFlay_Timer = 12000;                         //3sec channeled
        }

        
        if (ManaBurn_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(target && (target->getPowerType() == POWER_MANA))
                DoCast(target,SPELL_MANABURN);
            ManaBurn_Timer = 5000;                          //3 sec cast
        }

        if (ShadowWordPain_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(target)
            {
                DoCast(target,SPELL_SWPAIN);
                ShadowWordPain_Timer = 7000;
            }
        }
    }
};

#define SPELL_HAMMEROFJUSTICE       13005
#define SPELL_JUDGEMENTOFCOMMAND    29386
#define SPELL_SEALOFCOMMAND         29385

struct boss_baron_rafe_dreugerAI : public boss_moroes_guestAI
{
    //Retr Pally
    boss_baron_rafe_dreugerAI(Creature *c) : boss_moroes_guestAI(c){}

    Timer HammerOfJustice_Timer;
    Timer SealOfCommand_Timer;
    Timer JudgementOfCommand_Timer;

    void Reset()
    {
        HammerOfJustice_Timer.Reset(1000);
        SealOfCommand_Timer.Reset(7000);
        JudgementOfCommand_Timer = SealOfCommand_Timer.GetInterval() + 29000;

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (SealOfCommand_Timer.Expired(diff))
        {
            DoCast(me,SPELL_SEALOFCOMMAND);
            SealOfCommand_Timer = 32000;
            JudgementOfCommand_Timer = 29000;
        }

        
        if (JudgementOfCommand_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_JUDGEMENTOFCOMMAND);
            JudgementOfCommand_Timer = SealOfCommand_Timer.GetTimeLeft() + 29000;
        }

        
        if (HammerOfJustice_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_HAMMEROFJUSTICE);
            HammerOfJustice_Timer = 12000;
        }
    }
};

#define SPELL_DISPELMAGIC           15090                   //Self or other guest+Moroes
#define SPELL_GREATERHEAL           29564                   //Self or other guest+Moroes
#define SPELL_HOLYFIRE              29563
#define SPELL_PWSHIELD              29408

struct boss_lady_catriona_von_indiAI : public boss_moroes_guestAI
{
    //Holy Priest
    boss_lady_catriona_von_indiAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer DispelMagic_Timer;
    Timer GreaterHeal_Timer;
    Timer HolyFire_Timer;
    Timer PowerWordShield_Timer;

    void Reset()
    {
        DispelMagic_Timer.Reset(11000);
        GreaterHeal_Timer.Reset(1500);
        HolyFire_Timer.Reset(5000);
        PowerWordShield_Timer.Reset(1000);

        AcquireGUID();

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (PowerWordShield_Timer.Expired(diff))
        {
            DoCast(me,SPELL_PWSHIELD);
            PowerWordShield_Timer = 15000;
        }

        
        if (GreaterHeal_Timer.Expired(diff))
        {
            if (Unit* target = SelectLowestHpFriendly(50, 1000)) // spell heals target for 11-14k
                DoCast(target, SPELL_GREATERHEAL);
            GreaterHeal_Timer = 17000;
        }

        
        if (HolyFire_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HOLYFIRE);
            HolyFire_Timer = 22000;
        }

        
        if (DispelMagic_Timer.Expired(diff))
        {
            if(roll_chance_i(50))
            {
                std::vector<Unit*> DispellTargets;
                std::list<Creature*> DispellableTargetsAll = FindAllCreaturesWithDispellMask(30, 1 << 1);
                for(std::list<Creature *>::iterator i = DispellableTargetsAll.begin(); i != DispellableTargetsAll.end(); i++)
                {
                    if (Unit* target = Unit::GetUnit((*me), (*i)->GetGUIDLow()))
                    {
                        if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 17007 || target->GetEntry() == 19872 || target->GetEntry() == 19873 || target->GetEntry() == 19874 || target->GetEntry() == 19875 || target->GetEntry() == 19876 || target->GetEntry() == 15687)
                            DispellTargets.push_back(target);
                    }
                }

                Unit* target = !DispellTargets.empty() ? DispellTargets[urand(0, DispellTargets.size() -1)] : SelectTarget();
                if(target)
                    DoCast(target, SPELL_DISPELMAGIC);
            }
            else
                DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_DISPELMAGIC);

            DispelMagic_Timer = 25000;
        }
    }
};

#define SPELL_CLEANSE               29380                   //Self or other guest+Moroes
#define SPELL_GREATERBLESSOFMIGHT   29381                   //Self or other guest+Moroes
#define SPELL_HOLYLIGHT             29562                   //Self or other guest+Moroes
#define SPELL_DIVINESHIELD          41367

struct boss_lady_keira_berrybuckAI : public boss_moroes_guestAI
{
    //Holy Pally
    boss_lady_keira_berrybuckAI(Creature *c) : boss_moroes_guestAI(c)  {}

    Timer Cleanse_Timer;
    Timer GreaterBless_Timer;
    Timer HolyLight_Timer;
    Timer DivineShield_Timer;

    void Reset()
    {
        Cleanse_Timer.Reset(13000);
        GreaterBless_Timer.Reset(1000);
        HolyLight_Timer.Reset(7000);
        DivineShield_Timer.Reset(31000);

        AcquireGUID();

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (DivineShield_Timer.Expired(diff))
        {
            DoCast(me,SPELL_DIVINESHIELD);
            DivineShield_Timer = 31000;
        }

        if (HolyLight_Timer.Expired(diff))
        {
            if (Unit* target = SelectLowestHpFriendly(50, 1000))
                DoCast(target, SPELL_HOLYLIGHT);
            HolyLight_Timer = 10000;
        }

        if (GreaterBless_Timer.Expired(diff))
        {
            Unit* target = SelectTarget();

            DoCast(target, SPELL_GREATERBLESSOFMIGHT);

            GreaterBless_Timer = 50000;
        }

        if (Cleanse_Timer.Expired(diff))
        {
            std::vector<Unit*> DispellTargets;
            std::list<Creature*> DispellableTargetsAll = FindAllCreaturesWithDispellMask(30, 1 << 4 | 1 << 3 | 1 << 1);
            for(std::list<Creature *>::iterator i = DispellableTargetsAll.begin(); i != DispellableTargetsAll.end(); i++)
            {
                if (Unit* target = Unit::GetUnit((*me), (*i)->GetGUIDLow()))
                {
                    if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 17007 || target->GetEntry() == 19872 || target->GetEntry() == 19873 || target->GetEntry() == 19874 || target->GetEntry() == 19875 || target->GetEntry() == 19876 || target->GetEntry() == 15687)
                        DispellTargets.push_back(target);
                }
            }

            Unit* target = !DispellTargets.empty() ? DispellTargets[urand(0, DispellTargets.size() -1)] : SelectTarget();
            if(target)
                DoCast(target, SPELL_CLEANSE);

            Cleanse_Timer = 10000;
        }
    }
};

#define SPELL_HAMSTRING         9080
#define SPELL_MORTALSTRIKE      29572
#define SPELL_WHIRLWIND         29573

struct boss_lord_robin_darisAI : public boss_moroes_guestAI
{
    //Arms Warr
    boss_lord_robin_darisAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer Hamstring_Timer;
    Timer MortalStrike_Timer;
    Timer WhirlWind_Timer;

    void Reset()
    {
        Hamstring_Timer.Reset(7000);
        MortalStrike_Timer.Reset(10000);
        WhirlWind_Timer.Reset(21000);

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (Hamstring_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_HAMSTRING);
            Hamstring_Timer = 12000;
        }

        
        if (MortalStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_MORTALSTRIKE);
            MortalStrike_Timer = 18000;
        }

        
        if (WhirlWind_Timer.Expired(diff))
        {
            DoCast(me,SPELL_WHIRLWIND);
            WhirlWind_Timer = 21000;
        }
    }
};

#define SPELL_DISARM            8379
#define SPELL_HEROICSTRIKE      29567
#define SPELL_SHIELDBASH        11972
#define SPELL_SHIELDWALL        29390

struct boss_lord_crispin_ferenceAI : public boss_moroes_guestAI
{
    //Arms Warr
    boss_lord_crispin_ferenceAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer Disarm_Timer;
    Timer HeroicStrike_Timer;
    Timer ShieldBash_Timer;
    Timer ShieldWall_Timer;

    void Reset()
    {
        Disarm_Timer.Reset(6000);
        HeroicStrike_Timer.Reset(10000);
        ShieldBash_Timer.Reset(8000);
        ShieldWall_Timer.Reset(4000);

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (Disarm_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_DISARM);
            Disarm_Timer = 12000;
        }

        
        if (HeroicStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_HEROICSTRIKE);
            HeroicStrike_Timer = 10000;
        }

        
        if (ShieldBash_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SHIELDBASH);
            ShieldBash_Timer = 13000;
        }

        
        if (ShieldWall_Timer.Expired(diff))
        {
            DoCast(me,SPELL_SHIELDWALL);
            ShieldWall_Timer = 21000;
        }
    }
};

CreatureAI* GetAI_boss_moroes(Creature *_Creature)
{
    return new boss_moroesAI (_Creature);
}

CreatureAI* GetAI_baroness_dorothea_millstipe(Creature *_Creature)
{
    return new boss_baroness_dorothea_millstipeAI (_Creature);
}

CreatureAI* GetAI_baron_rafe_dreuger(Creature *_Creature)
{
    return new boss_baron_rafe_dreugerAI (_Creature);
}

CreatureAI* GetAI_lady_catriona_von_indi(Creature *_Creature)
{
    return new boss_lady_catriona_von_indiAI (_Creature);
}

CreatureAI* GetAI_lady_keira_berrybuck(Creature *_Creature)
{
    return new boss_lady_keira_berrybuckAI (_Creature);
}

CreatureAI* GetAI_lord_robin_daris(Creature *_Creature)
{
    return new boss_lord_robin_darisAI (_Creature);
}

CreatureAI* GetAI_lord_crispin_ference(Creature *_Creature)
{
    return new boss_lord_crispin_ferenceAI (_Creature);
}

void AddSC_boss_moroes()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_moroes";
    newscript->GetAI = &GetAI_boss_moroes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_baroness_dorothea_millstipe";
    newscript->GetAI = &GetAI_baroness_dorothea_millstipe;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_baron_rafe_dreuger";
    newscript->GetAI = &GetAI_baron_rafe_dreuger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lady_catriona_von_indi";
    newscript->GetAI = &GetAI_lady_catriona_von_indi;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lady_keira_berrybuck";
    newscript->GetAI = &GetAI_lady_keira_berrybuck;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lord_robin_daris";
    newscript->GetAI = &GetAI_lord_robin_daris;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lord_crispin_ference";
    newscript->GetAI = &GetAI_lord_crispin_ference;
    newscript->RegisterSelf();
}

