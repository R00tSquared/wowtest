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
SDName: Boss_Nefarian
SD%Complete: 99
SDComment: Debug, testing
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

enum Nefarian
{
    SAY_AGGRO                   = -1469007,
    SAY_XHEALTH                 = -1469008, // 5% hp
    SAY_SHADOWFLAME             = -1469009,
    SAY_RAISE_SKELETONS         = -1469010,
    SAY_SLAY                    = -1469011,
    SAY_DEATH                   = -1469012,

    SAY_MAGE                    = -1469013,
    SAY_WARRIOR                 = -1469014,
    SAY_DRUID                   = -1469015,
    SAY_PRIEST                  = -1469016,
    SAY_PALADIN                 = -1469017,
    SAY_SHAMAN                  = -1469018,
    SAY_WARLOCK                 = -1469019,
    SAY_HUNTER                  = -1469020,
    SAY_ROGUE                   = -1469021,
    
    SPELL_SHADOWFLAME_INITIAL   = 22992,
    SPELL_SHADOWFLAME           = 22539,
    SPELL_BELLOWINGROAR         = 22686,
    SPELL_VEILOFSHADOW          = 22687,
    SPELL_CLEAVE                = 20691,
    SPELL_TAILLASH              = 23364,
    SPELL_BONECONTRUST          = 23363, // 23362, 23361

    SPELL_MAGE                  = 23410,
    SPELL_WARRIOR               = 23397,
    SPELL_DRUID                 = 23398,
    SPELL_PRIEST                = 23401,
    SPELL_PALADIN               = 23418,
    SPELL_SHAMAN                = 23425,
    SPELL_WARLOCK               = 23427,
    SPELL_HUNTER                = 23436,
    SPELL_ROGUE                 = 23414,

    CREATURE_BONE_CONSTRUCT     = 14605
};
struct boss_nefarianAI : public ScriptedAI
{
    boss_nefarianAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
        me->SetAggroRange(100.0f);
    }

    ScriptedInstance * pInstance;

    Timer _ChangedNameShadowFlame_Timer;
    Timer _ChangedNameBellowingRoar_Timer;
    Timer _ChangedNameVeilOfShadow_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameTailLash_Timer;
    Timer _ChangedNameClassCall_Timer;
    uint32 LevitateTimer;

    bool Phase3;
    bool HasEndYell;
    bool onground;

    Timer _ChangedNameDespawnTimer;

    void Reset()
    {
        _ChangedNameShadowFlame_Timer  .Reset(12000);
        _ChangedNameBellowingRoar_Timer.Reset(30000);
        _ChangedNameVeilOfShadow_Timer .Reset(15000);
        _ChangedNameCleave_Timer       .Reset(7000);
        _ChangedNameTailLash_Timer     .Reset(10000);
        _ChangedNameClassCall_Timer    .Reset(urand(30000, 35000)); // 25-35 seconds
        _ChangedNameDespawnTimer       .Reset(5000); // 5 seconds after no victim - despawn, when victim appears - timer resets
        LevitateTimer       = 7000;
        onground            = false;
        Phase3              = false;
        HasEndYell          = false;

        me->SetLevitate(true);
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
        //me->addUnitState(UNIT_STAT_IGNORE_ATTACKERS);
        me->GetMotionMaster()->MovePoint(1, -7502.002, -1256.503, 476.758);

        if (pInstance && pInstance->GetData(DATA_NEFARIAN_EVENT) != DONE)
            pInstance->SetData(DATA_NEFARIAN_EVENT, NOT_STARTED);
    }

    void EnterEvadeMode()
    {
        if(pInstance)
            pInstance->SetData(DATA_NEFARIAN_EVENT, NOT_STARTED);
    }

    void KilledUnit(Unit* Victim)
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, me, Victim);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_NEFARIAN_EVENT, DONE);
    }

    void JustSummoned(Creature* summoned)
    {
        DoScriptText(SAY_AGGRO, me);
    }

    void EnterCombat(Unit* who)
    {
        DoCast(who, SPELL_SHADOWFLAME_INITIAL);
        DoZoneInCombat();
        DoScriptText(SAY_SHADOWFLAME, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (!onground)
            {
                if (LevitateTimer < diff)
                {
                    me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
                    //me->ClearUnitState(UNIT_STAT_IGNORE_ATTACKERS);
                    me->SetLevitate(false);
                    onground = true;
                    if (Unit* target = me->SelectNearestTarget(100.0f)) // start on nearest target
                    {
                        me->AI()->AttackStart(target);
                        me->AddThreat(target, 1000); // so we dont run off to totems or shit
                    }
                } 
                else 
                    LevitateTimer -= diff;
            }
            else
            {
                if(_ChangedNameDespawnTimer.Expired(diff))
                {
                    me->SetHealth(0);
                    me->setDeathState(JUST_DIED);
                    me->RemoveCorpse();
                }
            }
            return;
        }
        else
            _ChangedNameDespawnTimer.Reset(5000);

        // ShadowFlame_Timer
        if (_ChangedNameShadowFlame_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SHADOWFLAME);
            _ChangedNameShadowFlame_Timer = 12000;
        }
        

        if (_ChangedNameBellowingRoar_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_BELLOWINGROAR);
            _ChangedNameBellowingRoar_Timer = 30000;
        }
           

        if (_ChangedNameVeilOfShadow_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_VEILOFSHADOW);
            _ChangedNameVeilOfShadow_Timer = 15000;
        }
        

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 7000;
        }
        

        if (_ChangedNameTailLash_Timer.Expired(diff))
        {
            //Cast NYI since we need a better check for behind target
            //DoCast(me->GetVictim(),SPELL_TAILLASH);

            _ChangedNameTailLash_Timer = 10000;
        }
        

        if (_ChangedNameClassCall_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            switch (target->GetClass())
            {
                case CLASS_MAGE:
                    DoScriptText(SAY_MAGE, me);
                    DoCast(me, SPELL_MAGE);
                    break;
                case CLASS_WARRIOR:
                    DoScriptText(SAY_WARRIOR, me);
                    DoCast(me, SPELL_WARRIOR);
                    break;
                case CLASS_DRUID:
                    DoScriptText(SAY_DRUID, me);
                    DoCast(target, SPELL_DRUID);
                    break;
                case CLASS_PRIEST:
                    DoScriptText(SAY_PRIEST, me);
                    DoCast(me, SPELL_PRIEST);
                    break;
                case CLASS_PALADIN:
                    DoScriptText(SAY_PALADIN, me);
                    DoCast(me, SPELL_PALADIN);
                    break;
                case CLASS_SHAMAN:
                    DoScriptText(SAY_SHAMAN, me);
                    DoCast(me, SPELL_SHAMAN);
                    break;
                case CLASS_WARLOCK:
                    DoScriptText(SAY_WARLOCK, me);
                    DoCast(me, SPELL_WARLOCK);
                    break;
                case CLASS_HUNTER:
                    DoScriptText(SAY_HUNTER, me);
                    DoCast(me, SPELL_HUNTER);
                    break;
                case CLASS_ROGUE:
                    DoScriptText(SAY_ROGUE, me);
                    DoCast(me, SPELL_ROGUE);
                    break;
                default:
                    break;
            }

            _ChangedNameClassCall_Timer = urand(35000, 40000);
        }


        //Phase3 begins when we are below 20% health
        if (!Phase3 && (me->GetHealthPercent()) < 20)
        {
            std::list<Creature*> drakonidList= FindAllCreaturesWithEntry(CREATURE_BONE_CONSTRUCT, 500);
            for (std::list<Creature*>::iterator itr = drakonidList.begin(); itr != drakonidList.end(); ++itr)
            if ((*itr) && (*itr)->getStandState() == UNIT_STAND_STATE_DEAD)
            {
                // (*itr)->Respawn();
                (*itr)->setFaction(103);
                (*itr)->SetInCombatWithZone();
                (*itr)->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                (*itr)->SetReactState(REACT_AGGRESSIVE);
                (*itr)->SetStandState(UNIT_STAND_STATE_STAND);
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                    (*itr)->AI()->AttackStart(target);
            }
            Phase3 = true;
            DoScriptText(SAY_RAISE_SKELETONS, me);
        }

        // 5% hp yell
        if (!HasEndYell && (me->GetHealthPercent()) < 5)
        {
            HasEndYell = true;
            DoScriptText(SAY_XHEALTH, m_creature);
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_nefarian(Creature *_Creature)
{
    return new boss_nefarianAI (_Creature);
}

void AddSC_boss_nefarian()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_nefarian";
    newscript->GetAI = &GetAI_boss_nefarian;
    newscript->RegisterSelf();
}


