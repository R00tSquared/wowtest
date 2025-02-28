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
SDName: 
SD%Complete:
SDComment:
EndScriptData */

#include "precompiled.h"

const int32 opposedFactions[3][2] =
{
    { 47, 76 }, // Ironforge/Orgrimmar
    { 730, 729 }, // Stormpike/Frostwolf
    { 2310, 2316 }, // Stormwind/Undercity
};

enum
{
    QUEST_5892  = 5892, // A, 47 = 25, 730 = 2; 12096
    QUEST_5893  = 5893, // H, 76 = 25, 729 = 2; 12097
    
    QUEST_6741  = 6741, // H, 76 = 10, 729 = 1; 13176
    QUEST_6781  = 6781, // A, 47 = 10, 730 = 1; 13257
    
    QUEST_6801  = 6801, // H, 76 = 10, 729 = 1; 13236
    QUEST_6881  = 6881, // A, 47 = 10, 730 = 1; 13442

    QUEST_6825  = 6825, // H, 76 = 10, 729 = 1; 13179
    QUEST_6941  = 6941, // A, 47 = 10, 730 = 1; 13439

    QUEST_6826  = 6826, // H, 76 = 10, 729 = 2; 13180
    QUEST_6942  = 6942, // A, 47 = 10, 730 = 2; 13438 In fact gives 1, but its server bug.

    QUEST_6827  = 6827, // H, 76 = 10, 729 = 5; 13181
    QUEST_6943  = 6943, // A, 47 = 10, 730 = 5; 13437

    QUEST_6846  = 6846, // A, 47 = 10; 13446
    QUEST_6901  = 6901, // H, 76 = 10; If fact 0, bug. 13449

    QUEST_6982  = 6982, // A, 47 = 25, 730 = 2; 12096
    QUEST_6985  = 6985, // H, 76 = 25, 729 = 2; 12097

    QUEST_7001  = 7001, // H, 76 = 10(in fact 0, bug), 729 = 0; 13616
    QUEST_7027  = 7027, // A, 47 = 10, 730 = 0(in fact 10, bug); 13617

    QUEST_7002  = 7002, // H, 76 = 10, 729 = 2; 13441
    QUEST_7026  = 7026, // A, 47 = 10, 730 = 2; 13577

    QUEST_7081  = 7081, // A, 47 = 250, 730 = 250; 13777
    QUEST_7082  = 7082, // H, 76 = 250, 729 = 250; In fact 0, bug; 13776

    QUEST_7101  = 7101, // H, 76 = 250, 729 = 250. In fact 0, bug; 13776
    QUEST_7102  = 7102, // A, 47 = 250, 730 = 250; 13777

    QUEST_7121  = 7121, // A, 47 = 10, 730 = 10; 12096
    QUEST_7123  = 7123, // H, 76 = 10, 729 = 10; 12097

    QUEST_7122  = 7122, // A, 47 = 250, 730 = 250; 13777
    QUEST_7124  = 7124, // H, 76 = 250, 729 = 250. In fact 0, bug; 13776

    QUEST_7161  = 7161, // H, 76 = 250, 729 = 250; 13840
    QUEST_7162  = 7162, // A, 47 = 250, 730 = 250; 13841

    QUEST_7163  = 7163, // H, 76 = 250, 729 = 250; 13840
    QUEST_7168  = 7168, // A, 47 = 250(in fact 0, bug), 730 = 250; 13841

    QUEST_7164  = 7164, // H, 76 = 250, 729 = 250; 13840
    QUEST_7169  = 7169, // A, 47 = 250, 730 = 250; 13841

    QUEST_7165  = 7165, // H, 76 = 350, 729 = 350; 13840
    QUEST_7170  = 7170, // A, 47 = 350(0 - bug), 730 = 350; 13841

    QUEST_7166  = 7166, // H, 76 = 500, 729 = 500; 13840
    QUEST_7171  = 7171, // A, 47 = 500(0 - bug), 730 = 500; 13841

    QUEST_7167  = 7167, // H, 76 = 500, 729 = 500; 13840
    QUEST_7172  = 7172, // A, 47 = 500(0 - bug), 729 = 500; 13841

    QUEST_7223  = 7223, // A, 47 = 250, 730 = 250; 13257
    QUEST_7224  = 7224, // H, 76 = 250, 729 = 250(1 bug); 13176

    QUEST_7281  = 7281, // H, 76 = 250, 68 = 250, 729 = 250. rep 67 is bug, should be 68.; 13154
    QUEST_7282  = 7282, // A, 47 = 250, 72 = 250, 730 = 250. rep 469 is bug, should be 72.; 13320

    QUEST_7301  = 7301, // A, 47 = 250, 730 = 250; 13319
    QUEST_7302  = 7302, // H, 76 = 250, 729 = 250; 13153
    
    QUEST_7367  = 7367, // A, 47 = 250, 730 = 250; 0 - bug; 13598
    QUEST_7368  = 7368, // H, 76 = 250, 729 = 250; 0 - bug; 13957

    QUEST_7385  = 7385, // H, 76 = 75, 729 = 5; 13236
    QUEST_7386  = 7386, // A, 47 = 75, 730 = 5; 13442


// quests 8369, 6847, 6848, 6861, 6862, 7181, 7202, 7221, 7222, 7361, 7362, 7363, 7364, 7365, 7366, 7381, 7382, 7401, 7402, 7421, 7422, 7423, 7424, 7425, 7426, 7427, 7428 were removed from game, due to blizzards' restructurization of AV. Anyway, they are still exists in our database.
// Quests 7141, 7142, 7241, 7261, 8271, 8272, 11336, 11340, 51002, 51003, 51004, 51005 are out of AV. Check BG script if needed. Also 7142 is bugged - no rep.
    
};

bool QuestComplete_av_quest_script(Player *player, Creature *creature, const Quest *quest)
{
    uint32 reputations[3] = { 0, 0, 0 };

    switch(quest->GetQuestId())
    {
        case QUEST_5892:
        case QUEST_5893:
        case QUEST_6982:
        case QUEST_6985:
        {
            reputations[0] = 25;
            reputations[1] = 2;
            break;
        }
        case QUEST_6741:
        case QUEST_6781:
        case QUEST_6881:
        case QUEST_6801:
        case QUEST_6825:
        case QUEST_6941:
        case QUEST_7001:
        case QUEST_7027:
        {
            reputations[0] = 10;
            reputations[1] = 1;
            break;
        }
        case QUEST_6826:
        case QUEST_6942:
        case QUEST_7002:
        case QUEST_7026:
        {
            reputations[0] = 10;
            reputations[1] = 2;
            break;
        }
        case QUEST_6827:
        case QUEST_6943:
        {
            reputations[0] = 10;
            reputations[1] = 5;
            break;
        }
        case QUEST_6846:
        case QUEST_6901:
        {
            reputations[0] = 10;
            reputations[1] = 10;
            break;
        }
        case QUEST_7081:
        case QUEST_7082:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            break;
        }
        case QUEST_7101:
        case QUEST_7102:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7101 ? QUEST_7102 : QUEST_7101, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7121:
        case QUEST_7123:
        {
            reputations[0] = 10;
            reputations[1] = 10;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7121 ? QUEST_7123 : QUEST_7121, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7122:
        case QUEST_7124:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7122 ? QUEST_7124 : QUEST_7122, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7161:
        case QUEST_7162:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7161 ? QUEST_7162 : QUEST_7161, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7163:
        case QUEST_7168:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7163 ? QUEST_7168 : QUEST_7163, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7164:
        case QUEST_7169:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7164 ? QUEST_7169 : QUEST_7164, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7165:
        case QUEST_7170:
        {
            reputations[0] = 350;
            reputations[1] = 350;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7165 ? QUEST_7170 : QUEST_7165, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7166:
        case QUEST_7171:
        {
            reputations[0] = 500;
            reputations[1] = 500;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7166 ? QUEST_7171 : QUEST_7166, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7167:
        case QUEST_7172:
        {
            reputations[0] = 500;
            reputations[1] = 500;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7167 ? QUEST_7172 : QUEST_7167, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7223:
        case QUEST_7224:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7223 ? QUEST_7224 : QUEST_7223, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7281:
        case QUEST_7282:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            reputations[2] = 250;
            player->SetQuestStatus(quest->GetQuestId() == QUEST_7281 ? QUEST_7282 : QUEST_7281, QUEST_STATUS_COMPLETE);
            break;
        }
        case QUEST_7301:
        case QUEST_7302:
        case QUEST_7367:
        case QUEST_7368:
        {
            reputations[0] = 250;
            reputations[1] = 250;
            break;
        }
        case QUEST_7385:
        case QUEST_7386:
        {
            reputations[0] = 75;
            reputations[1] = 5;
            break;
        }
    }

    bool horde = player->GetTeam() == HORDE; // 0 for alliance, 1 for horde
    for (uint32 i = 0; i < 3; ++i)
    {
        if (!reputations[i])
            continue; // no rep

        int32 rep = player->CalculateReputationGain(REPUTATION_SOURCE_QUEST, reputations[i], opposedFactions[i][horde], player->GetQuestOrPlayerLevel(quest));
        player->GetReputationMgr().ModifyReputation(opposedFactions[i][horde], rep);
    }

    return true;
}

void AddSC_av_quest_scripts()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "av_questscript";
    newscript->pQuestRewardedNPC = &QuestComplete_av_quest_script;
    newscript->RegisterSelf();
}