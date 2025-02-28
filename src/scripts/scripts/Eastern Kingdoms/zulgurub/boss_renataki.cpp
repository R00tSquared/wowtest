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
SDName: Boss_Renataki
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SPELL_AMBUSH         24337
#define SPELL_THOUSANDBLADES 24649

struct boss_renatakiAI : public ScriptedAI
{
    boss_renatakiAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    Timer _ChangedNameInvisible_Timer;
    Timer _ChangedNameAmbush_Timer;
    Timer _ChangedNameVisible_Timer;
    Timer _ChangedNameAggro_Timer;
    Timer _ChangedNameThousandBlades_Timer;

    bool Invisible;
    bool Ambushed;

    ScriptedInstance * pInstance;

    void Reset()
    {
        _ChangedNameInvisible_Timer.Reset(8000 + rand()%10000);
        _ChangedNameAmbush_Timer.Reset(3000);
        _ChangedNameVisible_Timer.Reset(4000);
        _ChangedNameAggro_Timer.Reset(15000 + rand()%10000);
        _ChangedNameThousandBlades_Timer.Reset(4000 + rand()%4000);

        Invisible = false;
        Ambushed = false;

        pInstance->SetData(DATA_EDGEOFMADNESSEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        pInstance->SetData(DATA_EDGEOFMADNESSEVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        pInstance->SetData(DATA_EDGEOFMADNESSEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(_ChangedNameInvisible_Timer.Expired(diff))
        {
            m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
            m_creature->SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, 0);
            m_creature->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO , 218171138);
            m_creature->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO  + 1, 3);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,11686);
            Invisible = true;

            _ChangedNameInvisible_Timer = 15000 + rand()%15000;
        }
        

        if(Invisible)
        {
            if(_ChangedNameAmbush_Timer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                {
                    DoTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                    DoCast(target,SPELL_AMBUSH);
                }

                Ambushed = true;
                _ChangedNameAmbush_Timer = 3000;
            }
        }

        if (Ambushed)
        {
            if (_ChangedNameVisible_Timer.Expired(diff))
            {
                m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
                m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,15268);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, 31818);
                m_creature->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO , 218171138);
                m_creature->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO  + 1, 3);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                Invisible = false;

                _ChangedNameVisible_Timer = 4000;
            }
        }

        //Resetting some aggro so he attacks other gamers
        if(!Invisible)
        {
            if(_ChangedNameAggro_Timer.Expired(diff))
            {
                Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1, 200, true, m_creature->getVictimGUID());
                if(DoGetThreat(m_creature->GetVictim()))
                    DoModifyThreatPercent(m_creature->GetVictim(),-50);

                if(target)
                    AttackStart(target);

                _ChangedNameAggro_Timer = 7000 + rand()%13000;
            }
            

            if(_ChangedNameThousandBlades_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_THOUSANDBLADES);
                _ChangedNameThousandBlades_Timer = 7000 + rand()%5000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_renataki(Creature *_Creature)
{
    return new boss_renatakiAI (_Creature);
}

void AddSC_boss_renataki()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_renataki";
    newscript->GetAI = &GetAI_boss_renataki;
    newscript->RegisterSelf();
}


