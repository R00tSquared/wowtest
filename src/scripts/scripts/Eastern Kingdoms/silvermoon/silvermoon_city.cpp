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
SDName: Silvermoon_City
SD%Complete: 100
SDComment: Quest support: 9685
SDCategory: Silvermoon City
EndScriptData */

/* ContentData
npc_blood_knight_stillblade
npc_silvermoon_guard
EndContentData */

#include "precompiled.h"

/*#######
# npc_blood_knight_stillblade
#######*/

#define SAY_HEAL -1000334

#define QUEST_REDEEMING_THE_DEAD        9685
#define SPELL_SHIMMERING_VESSEL         31225
#define SPELL_REVIVE_SELF               32343

struct npc_blood_knight_stillbladeAI : public ScriptedAI
{
    npc_blood_knight_stillbladeAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked lifeTimer;
    bool spellHit;

    void Reset()
    {
        lifeTimer.Reset(120000);
        me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        spellHit = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void MoveInLineOfSight(Unit *who)
    {
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->GetUInt32Value(UNIT_FIELD_BYTES_1))
        {
            if(lifeTimer.Expired(diff))
            {
                m_creature->AI()->EnterEvadeMode();
                return;
            }
        }
    }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if((Spellkind->Id == SPELL_SHIMMERING_VESSEL) && !spellHit &&
            (Hitter->GetTypeId() == TYPEID_PLAYER) && (((Player*)Hitter)->IsActiveQuest(QUEST_REDEEMING_THE_DEAD)))
        {
            ((Player*)Hitter)->AreaExploredOrEventHappens(QUEST_REDEEMING_THE_DEAD);
            DoCast(m_creature,SPELL_REVIVE_SELF);
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
            me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            DoScriptText(SAY_HEAL, m_creature);
            spellHit = true;
        }
    }
};

CreatureAI* GetAI_npc_blood_knight_stillblade(Creature *_Creature)
{
    return new npc_blood_knight_stillbladeAI (_Creature);
}




void AddSC_silvermoon_city()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="npc_blood_knight_stillblade";
    newscript->GetAI = &GetAI_npc_blood_knight_stillblade;
    newscript->RegisterSelf();
}

