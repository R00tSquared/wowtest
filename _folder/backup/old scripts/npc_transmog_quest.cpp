// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"

bool QuestAccept_npc_transmog_quest(Player* player, Creature* creature, Quest const* quest)
{
    int32 bp = 0;
    int32 bp2 = 0;
    switch (quest->GetQuestId())
    {
        case 93000:
            bp = 1050000 + urand(0,127);
            break;
        case 93001:
            bp = 1051000 + urand(0,423);
            break;
        case 93002:
            bp = 1052000 + urand(0,215);
            break;
        case 93003:
            bp = 1053000 + urand(0,194);
            break;
        case 93004:
            bp = 1054000 + urand(0,625);
            break;
        default:
            break;
    }
    if (bp)
    {
        bp2 = bp + 5000;
        player->CastCustomSpell(player, 55151, &bp, &bp2, NULL, true);
    }
    return true;
}

bool QuestAccept_npc_transmog_quest_complete(Player *player, Creature *creature, const Quest *_Quest)
{
    if (player->HasAura(55151))
        player->RemoveAurasDueToSpell(55151);

    return true;
}

 void AddSC_npc_transmog_quest()
 {
     Script *newscript;
     newscript = new Script;
     newscript->Name = "npc_transmog_quest";
     newscript->pQuestRewardedNPC = &QuestAccept_npc_transmog_quest_complete;
     newscript->pQuestAcceptNPC = &QuestAccept_npc_transmog_quest;
     newscript->RegisterSelf();
 }
