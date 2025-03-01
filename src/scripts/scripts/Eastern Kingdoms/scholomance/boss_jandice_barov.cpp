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
SDName: Boss_jandicebarov
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"

#define SPELL_CURSEOFBLOOD          24673

//#define SPELL_BANISH                8994
//#define SPELL_ILLUSION              17773

struct boss_jandicebarovAI : public ScriptedAI
{
    boss_jandicebarovAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCurseOfBlood_Timer;
    Timer _ChangedNameIllusion_Timer;
    //int32 Illusioncounter;
    Timer _ChangedNameInvisible_Timer;
    bool Invisible;
    int Rand;
    int RandX;
    int RandY;

    void Reset()
    {
        _ChangedNameCurseOfBlood_Timer.Reset(15000);
        _ChangedNameIllusion_Timer.Reset(30000);
        _ChangedNameInvisible_Timer.Reset(3000);                             //Too much too low?
        Invisible = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void SummonIllusions(Unit* victim)
    {
        Rand = rand()%10;
        switch (rand()%2)
        {
        case 0: RandX = 0 - Rand; break;
        case 1: RandX = 0 + Rand; break;
        }
        Rand = 0;
        Rand = rand()%10;
        switch (rand()%2)
        {
        case 0: RandY = 0 - Rand; break;
        case 1: RandY = 0 + Rand; break;
        }
        Rand = 0;
        if(Creature* Summoned = DoSpawnCreature(11439, RandX, RandY, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000))
            ((CreatureAI*)Summoned->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Invisible)
        {
            if (_ChangedNameInvisible_Timer.Expired(diff))
            {
                //Become visible again
                m_creature->setFaction(14);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 11073);     //Jandice Model
                Invisible = false;
            }
                //Do nothing while invisible
                return;
            
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameCurseOfBlood_Timer.Expired(diff))
        {
            //Cast
            DoCast(m_creature->GetVictim(),SPELL_CURSEOFBLOOD);

            //45 seconds
            _ChangedNameCurseOfBlood_Timer = 30000;
        }

        if (_ChangedNameIllusion_Timer.Expired(diff))
        {

            //Inturrupt any spell casting
            m_creature->InterruptNonMeleeSpells(false);
            m_creature->setFaction(35);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,11686);  // Invisible Model
            DoModifyThreatPercent(m_creature->GetVictim(),-99);

            //Summon 10 Illusions attacking random gamers
            Unit* target = NULL;
            for(int i = 0; i < 10;i++)
            {
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if(target)
                    SummonIllusions(target);
            }
            Invisible = true;
            _ChangedNameInvisible_Timer = 3000;

            //25 seconds until we should cast this agian
            _ChangedNameIllusion_Timer = 25000;
        }


        //            if (_ChangedNameIllusion_Timer.Expired(diff))
        //            {
        //                  //Cast
        //                DoCast(m_creature->GetVictim(),SPELL_ILLUSION);
        //
        //                  //3 Illusion will be summoned
        //                  if (Illusioncounter < 3)
        //                  {
        //                    _ChangedNameIllusion_Timer = 500;
        //                    Illusioncounter++;
        //                  }
        //                  else {
        //                      //15 seconds until we should cast this again
        //                      _ChangedNameIllusion_Timer = 15000;
        //                      Illusioncounter=0;
        //                  }
        //            }

        DoMeleeAttackIfReady();
    }
};

// Illusion of Jandice Barov Script

struct mob_illusionofjandicebarovAI : public ScriptedAI
{
    mob_illusionofjandicebarovAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};


CreatureAI* GetAI_boss_jandicebarov(Creature *_Creature)
{
    return new boss_jandicebarovAI (_Creature);
}

CreatureAI* GetAI_mob_illusionofjandicebarov(Creature *_Creature)
{
    return new mob_illusionofjandicebarovAI (_Creature);
}


void AddSC_boss_jandicebarov()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_jandice_barov";
    newscript->GetAI = &GetAI_boss_jandicebarov;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_illusionofjandicebarov";
    newscript->GetAI = &GetAI_mob_illusionofjandicebarov;
    newscript->RegisterSelf();
}


