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
SDName: Duskwood
SD%Complete: 100
SDComment: Quest Support:8735
SDCategory: Duskwood
EndScriptData */

#include "precompiled.h"

#define BOSS_TWILIGHT_CORRUPTER            15625

bool CorrupterSummoned = false;

/*######
# at_twilight_grove
######*/

bool AreaTrigger_at_twilight_grove(Player* pPlayer, AreaTriggerEntry const* at)
{
    if (pPlayer->HasQuestForItem(21149) && !CorrupterSummoned)
    {
        Unit* TCorrupter = pPlayer->SummonCreature(BOSS_TWILIGHT_CORRUPTER,-10328.16,-489.57,49.95,0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,120000);
        if(TCorrupter)
        {
            TCorrupter->setFaction(14);
            TCorrupter->SetMaxHealth(832750 * TCorrupter->ToCreature()->GetCreatureHealthMod());
            CorrupterSummoned = true;
        }
        Unit* CorrupterSpeaker = pPlayer->SummonCreature(1,pPlayer->GetPositionX(),pPlayer->GetPositionY(),pPlayer->GetPositionZ()-1,0,TEMPSUMMON_TIMED_DESPAWN,15000);
        if(CorrupterSpeaker)
        {
            CorrupterSpeaker->SetName("Twilight Corrupter"); //TEXT_TODO -1200203
            CorrupterSpeaker->SetVisibility(VISIBILITY_ON);
            CorrupterSpeaker->MonsterYell(-1200204,0,pPlayer->GetGUID());
        }
    }
    return false;
};

/*######
# boss_twilight_corrupter
######*/

#define SPELL_SOUL_CORRUPTION           25805
#define SPELL_CREATURE_OF_NIGHTMARE     25806
#define SPELL_LEVEL_UP                  24312

struct boss_twilight_corrupterAI : public ScriptedAI
{
    boss_twilight_corrupterAI(Creature *c) : ScriptedAI(c) {}

    uint32 SoulCorruption_Timer;
    uint32 CreatureOfNightmare_Timer;
    uint8 KillCount;
    uint32 DespawnTimer;

    void Reset()
    {
        SoulCorruption_Timer = 15000;
        CreatureOfNightmare_Timer = 30000;
        KillCount = 0;
        DespawnTimer = 1680000;
        me->setActive(true);
    }

    void EnterCombat(Unit* who)
    {
        m_creature->MonsterYell(-1200205,0,m_creature->GetGUID());
    }

    void JustDied(Unit* Killer)
    {
        CorrupterSummoned = false;
        me->NearTeleportTo(-10328.16, -489.57, 50.0, 0);

        Player* player = Killer->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player)
            return;

        if (Group *grp = player->GetGroup())
        {
            for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *pGroupMember = itr->getSource();
        
                if (!pGroupMember || !pGroupMember->GetSession())
                    return;
        
                if (!pGroupMember->IsAtGroupRewardDistance(me))
                    return;

                if (pGroupMember->IsBeingTeleported())
                    return;

                // stop flight if need
                pGroupMember->InterruptTaxiFlying();

                pGroupMember->TeleportTo(0, -10328.16, -489.57, 50.0, 0);
            }
        }
    }

    void KilledUnit(Unit* victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
        {
            ++KillCount;
            m_creature->MonsterTextEmote(-1200206, victim->GetGUID(),true);

            if (KillCount == 3)
            {
                DoCast(m_creature, SPELL_LEVEL_UP, true);
                KillCount = 0;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(DespawnTimer <= diff)
            {
                me->Kill(me);
                DespawnTimer = 0;
            } else DespawnTimer -= diff;

            return;
        }

        if(SoulCorruption_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_SOUL_CORRUPTION);
            SoulCorruption_Timer = rand()%4000+15000;
        } else SoulCorruption_Timer-=diff;

        if(CreatureOfNightmare_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_CREATURE_OF_NIGHTMARE);
            CreatureOfNightmare_Timer = 45000;
        } else CreatureOfNightmare_Timer-=diff;

        DoMeleeAttackIfReady();
    };
};
CreatureAI* GetAI_boss_twilight_corrupter(Creature* pCreature)
{
    return new boss_twilight_corrupterAI (pCreature);
}

bool QuestComplete_npc_263(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 252)
    {
        Unit* stitches = FindCreature(412, 300.0, creature);
        if(!stitches)
        {
            if(Creature* stitches = creature->SummonCreature(412, -10720.489, -1170.776, 26.602, 5.8, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000))
                stitches->GetMotionMaster()->MovePath(4120, false);
        }
    }
    return true;
}


void AddSC_duskwood()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_twilight_corrupter";
    newscript->GetAI = &GetAI_boss_twilight_corrupter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_263";
    newscript->pQuestRewardedNPC = &QuestComplete_npc_263;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="at_twilight_grove";
    newscript->pAreaTrigger = &AreaTrigger_at_twilight_grove;
    newscript->RegisterSelf();
}
