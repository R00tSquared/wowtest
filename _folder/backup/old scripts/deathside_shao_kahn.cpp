// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"
#include "Chat.h"

#define SCRIPT_TEXT_15_SEC_TO_CHESTS -1901000

const float chestCoords[9][4] = 
{
    { -13198.6f, 262.613f, 21.8587f, 4.25f },
    { -13208.4f, 267.407f, 21.8583f, 4.25f },
    { -13217.7f, 271.971f, 21.8574f, 4.25f },
    { -13209.0f, 289.802f, 21.8577f, 4.25f },
    { -13189.7f, 280.349f, 21.8579f, 4.25f },
    { -13199.6f, 285.179f, 21.8579f, 4.25f },
    { -13213.2f, 281.388f, 21.8582f, 4.25f },
    { -13203.9f, 276.671f, 21.8574f, 4.25f },
    { -13194.0f, 271.842f, 21.8572f, 4.25f }
};

struct DeathSide_Shao_Kahn_AI : public ScriptedAI
{
    DeathSide_Shao_Kahn_AI(Creature* c) : ScriptedAI(c){}

    Timer chestSpawn; // 9 chests at the same time
    uint64 chestGUID[9];
    Timer checkRaidGroups;
    bool warn;
    void Reset()
    {
        warn = true;
        chestSpawn.Reset((3 * MINUTE * MILLISECONDS) - (15 * MILLISECONDS)); // 2 min 45 sec until warn
        for (uint32 i = 0; i < 9; ++i)
            chestGUID[i] = 0;
        checkRaidGroups.Reset(20000); // check every 20 sec for raid-groups and make players leave them!
    }

    void UpdateAI(const uint32 diff)
    {
        if (chestSpawn.Expired(diff))
        {
            if (warn)
            {
                warn = false;
                std::list<Player*> playerList = FindAllPlayersInRange(300); // get everyone
                playerList.remove_if([this](Player* plr)-> bool { return !plr->IsInGurubashiEvent(); });
                me->MonsterMessageToList(SCRIPT_TEXT_15_SEC_TO_CHESTS, playerList, CHAT_MSG_RAID_BOSS_WHISPER);

                chestSpawn = 15 * MILLISECONDS; // 15 sec until spawn
            }
            else
            {
                warn = true;
                GameObject *chest;
                for (uint32 i = 0; i < 9; ++i)
                {
                    chest = GameObject::GetGameObject(*me, chestGUID[i]);
                    if (chest)
                        continue;

                    chest = me->SummonGameObject(693102, chestCoords[i][0], chestCoords[i][1], chestCoords[i][2], chestCoords[i][3], 0, 0, 0, 0, 3600000); // An hour
                    if (chest)
                        chestGUID[i] = chest->GetGUID();
                }

                chestSpawn = ((3 * MINUTE * MILLISECONDS) - (15 * MILLISECONDS)); // 2 min 45 sec until warn
            }
        }

        if (checkRaidGroups.Expired(diff))
        {
            std::list<Player*> playerList = FindAllPlayersInRange(300); // get everyone
            // remove those who are not in event, or not in group, or not in a raid group from the 'leave list'
            playerList.remove_if([this](Player* plr)-> bool { return !plr->IsInGurubashiEvent() || !plr->GetGroup()/* || (!plr->GetGroup()->isRaidGroup())*/; });

            for (std::list<Player*>::iterator i = playerList.begin(); i != playerList.end(); ++i)
            {
                (*i)->RemoveFromGroup();
                ChatHandler(*i).SendSysMessage(LANG_EVENT_GURUBASHI_NO_RAID);
            }
            checkRaidGroups = 20000;
        }
    }
};

CreatureAI* GetAI_DeathSide_Shao_Kahn_AI(Creature* pCreature)
{
    return new DeathSide_Shao_Kahn_AI (pCreature);
}

void AddSC_DeathSide_Shao_Kahn()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "deathside_shao_kahn";
     newscript->GetAI = &GetAI_DeathSide_Shao_Kahn_AI;
     newscript->RegisterSelf();
 }
