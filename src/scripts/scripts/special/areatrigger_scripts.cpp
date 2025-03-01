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
SDName: Areatrigger_Scripts
SD%Complete: ?
SDComment: Scripts for areatriggers
SDCategory: Areatrigger
EndScriptData */

/* ContentData
at_legion_teleporter    4560 Teleporter TO Invasion Point: Cataclysm
at_scent_larkorwi       quest 4291
at_nats_landing         quest 11209

at_test                 script test only
EndContentData */

#include "precompiled.h"

/*#####
## at_legion_teleporter
#####*/

#define SPELL_TELE_A_TO   37387
#define SPELL_TELE_H_TO   37389

bool AreaTrigger_at_legion_teleporter(Player *player, AreaTriggerEntry const* at)
{
    if (player->isAlive() && !player->IsInCombat())
    {
        if (player->GetTeam()== ALLIANCE && player->GetQuestRewardStatus(10589))
        {
            player->CastSpell(player,SPELL_TELE_A_TO,false);
            return true;
        }

        if (player->GetTeam()== HORDE && player->GetQuestRewardStatus(10604))
        {
            player->CastSpell(player,SPELL_TELE_H_TO,false);
            return true;
        }

        return false;
    }
    return false;
}

/*######
## at_scent_larkorwi
######*/

enum eScentLarkorwi
{
    QUEST_SCENT_OF_LARKORWI               = 4291,
    NPC_LARKORWI_MATE                     = 9683
};

bool AreaTrigger_at_scent_larkorwi(Player* player, AreaTriggerEntry const* trigger)
{
    if (!player->isDead() && player->GetQuestStatus(QUEST_SCENT_OF_LARKORWI) == QUEST_STATUS_INCOMPLETE)
    {
        Unit* LarkorwiMate = FindCreature(NPC_LARKORWI_MATE, 15.0, player);
            if(!LarkorwiMate)
                player->SummonCreature(NPC_LARKORWI_MATE, player->GetPositionX()+5, player->GetPositionY(), player->GetPositionZ(), 3.3, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100000);
    }

    return false;
}

/*######
## at_nats_landing
######*/

enum eNatBargain
{
    QUEST_NATS_BARGAIN          = 11209,
    AURA_PAGLE_FISH_PASTE       = 42644,
    NPC_LURKING_SHARK           = 23928
};
float SharkPos[3] =
{
    -4246.243,
    -3922.356,
    -7.488
};

bool AreaTrigger_at_nats_landing(Player* player, AreaTriggerEntry const* trigger)
{
    if (player->GetQuestStatus(QUEST_NATS_BARGAIN) == QUEST_STATUS_INCOMPLETE && player->HasAura(AURA_PAGLE_FISH_PASTE, 0))
    {
        Unit* shark = FindCreature(NPC_LURKING_SHARK, 20.0, player);
        if(shark)
        {
            ((Creature*)shark)->AI()->AttackStart(player);
            return false;
        }
        else
        {
            Creature* Shark = player->SummonCreature(NPC_LURKING_SHARK, SharkPos[0], SharkPos[1], SharkPos[2], 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100000);
            Shark->AI()->AttackStart(player);
            return true;
        }
    }
    return false;
}

/*#####
## at_eredar_twins
#####*/

bool AreaTrigger_at_eredar_twins(Player* player, AreaTriggerEntry const* trigger)
{
    Unit* Alythess = FindCreature(25166, 50, player);
    if(Alythess)
        Alythess->ToCreature()->AI()->SetData(1, 1);    // just to trigger intro event
    return false;
}

bool AreaTrigger_at_arena(Player *player, AreaTriggerEntry const* at)
{
    BattleGround* bg = NULL;
    if (player)
        bg = player->GetBattleGround();
    if (player && player->isAlive() && bg && bg->isRated() && bg->GetStatus() == STATUS_IN_PROGRESS) // add check for falling flag
    {
        float x, y, z;
        player->GetPosition(x, y, z);
        player->UpdateAllowedPositionZ_WithUndermap(x, y, z); // be everywhere
        TerrainInfo const* map = player->GetTerrain();
        float z_pos = map->GetHeight(player->GetPositionX(), player->GetPositionY(), MAX_HEIGHT, false); // be: bridge case, nagrand everywhere
        float ground_z = map->GetHeight(player->GetPositionX(), player->GetPositionY(), MAX_HEIGHT); // for lordaeron. This also good for BE stairs
        // add different cases for different maps
        if (z - z_pos > 5.0f)
        {
            switch (player->GetMapId())
            {
            case 562: // blades edge arena
                if(ground_z  - z_pos >= 10.0f && (player->GetPositionX() <= 6268.0f || player->GetPositionX() >= 6211.0f))
                {
                    player->NearTeleportTo(x, y, z_pos + 1.0f, player->GetOrientation());
                    sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, z_pos + 1.0f);
                }
                else
                {
                    player->NearTeleportTo(x, y, ground_z + 1.0f, player->GetOrientation());
                    sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, ground_z + 1.0f);
                }
                
                break;
            // fun fact: lordaeron is piece of shit
            case 572: // lordaeron
                player->NearTeleportTo(x, y, ground_z, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, ground_z);
                break;
            case 559: // nagrand
                player->NearTeleportTo(x, y, z_pos + 0.4f, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, z_pos + 0.4f);
                break;
            default:
                player->NearTeleportTo(x, y, z_pos + 2.0f, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, z_pos + 2.0f);
                break;
            }
        }
        else
        {
            switch (player->GetMapId())
            {
            case 562: // blades edge arena
                player->NearTeleportTo(x, y, z + 2.0f, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, z + 2.0f);
                break;
            case 572: // lordaeron
                player->NearTeleportTo(x, y, ground_z, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, ground_z);
                break;
            case 559: // nagrand
                player->NearTeleportTo(x, y, z + 1.0f, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, z + 1.0f);
                break;
            default:
                player->NearTeleportTo(x, y, z + 2.0f, player->GetOrientation());
                sLog.outLog(LOG_DEFAULT, "Unhandled areatrigger: teleported to PX: %f, PY: %f, PZ: %f", x, y, z + 2.0f);
                break;
            }
        }
    }
    return false;
}

void AddSC_areatrigger_scripts()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "at_legion_teleporter";
    newscript->pAreaTrigger = &AreaTrigger_at_legion_teleporter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_scent_larkorwi";
    newscript->pAreaTrigger = &AreaTrigger_at_scent_larkorwi;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_nats_landing";
    newscript->pAreaTrigger = &AreaTrigger_at_nats_landing;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_eredar_twins";
    newscript->pAreaTrigger = &AreaTrigger_at_eredar_twins;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_arena_trigger";
    newscript->pAreaTrigger = &AreaTrigger_at_arena;
    newscript->RegisterSelf();
}

