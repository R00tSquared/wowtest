// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*####
## Game Event: Midsummer Fire Festival
## ID: 1
## Date: Year/06/21 22:00:00 - Year/12/31 00:00:00
## % done: Unknown
## Comment: Almost fully not working event.
####*/

#include "precompiled.h"

/*####
## Quest: Torch Tossing, More Torch Tossing
## Alliance ID: 11731, 11921
## Horde ID: 11922, 11926
## Comment: Missing spawns for Exodar and Silvermoon
####*/

enum qTorchTossing
{
    // NPCs
    N_TORCH_TOSSING_TARGET                = 25535,
    // Spells
    S_PRACTICE_TORCHES_I                  = 45732,
    S_TORCH_TOSSING_PRACTICE              = 46630, // Casted on player on More Torch Tossing
    S_TORCH_TOSSING_TRAINING              = 45716, // Should be caster on q accepting
    S_TORCH_TOSSING_TRAINING_SUCCESS_OK_A = 45719, // Makes quest completed for alliance
    S_TORCH_TOSSING_TRAINING_SUCCESS_OK_H = 46651, // Makes quest completed for horde
    S_TORCH_TOSSING_FAILURE               = 46070, // makes quest failed
    S_TARGET_MARK                         = 40790,
    S_TORCHES_CAUGHT                      = 45724,
    // Quests
    Q_TORCH_TOSSING_A                     = 11731, // Alliance ver. of quest
    Q_TORCH_TOSSING_H                     = 11922, // Horde ver. of quest
    Q_MORE_TORCH_TOSSING_A                = 11921,
    Q_MORE_TORCH_TOSSING_H                = 11926,
    // Zones
    Z_DARNASSUS                           = 141,
    Z_IRONFORGE                           = 1537,
    Z_STORMWIND                           = 1519,
    Z_ORGRIMMAR                           = 1637,
    Z_THUNDER_BLUFF                       = 1638,
    Z_UNDERCITY                           = 1497,
    Z_SILVEMOON                           = 3487,
    Z_EXODAR                              = 3557
};

float DarnassusLocations[5][4] = 
{
    {8716.730469, 936.471008, 14.8964, -1.58825},
    {8716.849609, 928.882996, 15.3478, -1.76278},
    {8717.299805, 920.104004, 15.1784, -1.98968},
    {8721.219727, 923.778992, 16.4874, 0.698132},
    {8722.040039, 933.661987, 15.9977, -2.18166}
};

float IronforgeLocations[5][4] = 
{
    {-4685.950195, -1218.959961, 501.658997, 0.698132},
    {-4678.689941, -1219.430054, 501.658997, -2.18166},
    {-4675.410156, -1224.660034, 501.658997, 1.98968 },
    {-4677.390137, -1229.829956, 501.658997, -1.58825},
    {-4683.979980, -1232.640015, 501.658997, -1.76278}
};

float StormwindLocations[5][4] = 
{
    {-8825.719727, 845.612976, 99.051102, 0.698132},
    {-8819.459961, 848.505981, 98.948303, -2.18166},
    {-8818.01,     865.253,    98.9761,   4.47084 },
    {-8815.11,     860.487,    98.96,     4.75751 },
    {-8816.54,     854.183,    98.882,    4.34518}
};

float OrgrimmarLocations[5][4] = 
{
    {1915.579956, -4320.459961, 21.8202, 0.698132},
    {1920.489990, -4319.350098, 21.8167, -2.18166},
    {1925.150024, -4321.270020, 21.654699, 1.98968},
    {1923.869995, -4315.270020, 22.4918, -1.58825},
    {1918.069946, -4314.899902, 22.856199, -1.76278}
};

float ThunderBluffLocations[5][4] = 
{
    {-1048.800049, 299.889008, 134.401001, 0.698132},
    {-1042.520020, 306.559998, 134.451004, -2.18166},
    {-1035.880005, 312.549011, 134.666, 1.98968},
    {-1041.609985, 313.162994, 133.278, -1.58825},
    {-1049.079956, 306.372986, 132.936996, -1.76278}
};

float UndercityLocations[5][4] = 
{
    {1837.189941, 225.626999, 60.245998, 0.698132},
    {1838.229980, 218.968994, 60.149601, -2.18166},
    {1837.400024, 213.158005, 60.3433, 1.98968},
    {1840.839966, 216.244995, 60.074001, -1.58825},
    {1840.180054, 222.606995, 60.206902, -1.76278}
};

float SilvermoonLocations[5][4] = 
{
    {9817.67, -7227.84, 26.1104, 3.89843},
    {9810.38, -7226.95, 26.0582, 0.898206},
    {9819.87, -7234.51, 26.1176, 0.898206},
    {9823.56, -7229.12, 26.1209, 3.78847},
    {9817.55, -7221.36, 26.1142, 3.80025}
};

float ExodarLocations[5][4] = 
{
    {-3768.73, -11511.2, -134.479, 2.70805},
    {-3773.26, -11519.4, -134.56, 2.53526},
    {-3776.32, -11511.5, -134.569, 5.69256},
    {-3775.23, -11506.7, -134.539, 5.79074},
    {-3780.47, -11514.1, -134.626, 5.66115}
};

struct npc_torch_tossing_targetAI : public ScriptedAI
{
    npc_torch_tossing_targetAI(Creature *c) : ScriptedAI(c) {}

    uint32 TeleportTimer;

    void Reset()
    {
        TeleportTimer = 5000;
        me->CastSpell(me, S_TARGET_MARK, true);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (caster->GetTypeId() == TYPEID_PLAYER && spell->Id == S_PRACTICE_TORCHES_I)
        {
            if(((Player*)caster)->GetQuestStatus(Q_TORCH_TOSSING_A) == QUEST_STATUS_INCOMPLETE || ((Player*)caster)->GetQuestStatus(Q_TORCH_TOSSING_H) == QUEST_STATUS_INCOMPLETE)
            {
                if(caster->HasAura(S_TORCHES_CAUGHT, 0))
                {
                    if(Aura* aura = caster->GetAura(S_TORCHES_CAUGHT, 0))
                    {
                        if(!aura || aura->GetStackAmount() <= 6)
                            me->CastSpell(caster, S_TORCHES_CAUGHT, true);
                        else
                        {
                            caster->RemoveAurasDueToSpell(S_TORCHES_CAUGHT);
                            if(((Player*)caster)->GetTeam() == ALLIANCE)
                            {
                                me->CastSpell(caster, S_TORCH_TOSSING_TRAINING_SUCCESS_OK_A, true);
                                ((Player*)caster)->AreaExploredOrEventHappens(Q_TORCH_TOSSING_A);
                            }
                            else if(((Player*)caster)->GetTeam() == HORDE)
                            {
                                me->CastSpell(caster, S_TORCH_TOSSING_TRAINING_SUCCESS_OK_H, true);
                                ((Player*)caster)->AreaExploredOrEventHappens(Q_TORCH_TOSSING_H);
                            }
                        }
                    }
                }
                else
                    me->CastSpell(caster, S_TORCHES_CAUGHT, true);
                TeleportTimer = 1000;
            }
            else if(((Player*)caster)->GetQuestStatus(Q_MORE_TORCH_TOSSING_A) == QUEST_STATUS_INCOMPLETE || ((Player*)caster)->GetQuestStatus(Q_MORE_TORCH_TOSSING_H) == QUEST_STATUS_INCOMPLETE)
            {
                if(caster->HasAura(S_TORCHES_CAUGHT, 0))
                {
                    if(Aura* aura = caster->GetAura(S_TORCHES_CAUGHT, 0))
                    {
                        if(!aura || aura->GetStackAmount() <= 18)
                            me->CastSpell(caster, S_TORCHES_CAUGHT, true);
                        else
                        {
                            caster->RemoveAurasDueToSpell(S_TORCHES_CAUGHT);
                            if(((Player*)caster)->GetTeam() == ALLIANCE)
                            {
                                me->CastSpell(caster, S_TORCH_TOSSING_TRAINING_SUCCESS_OK_A, true);
                                ((Player*)caster)->AreaExploredOrEventHappens(Q_MORE_TORCH_TOSSING_A);
                            }
                            else if(((Player*)caster)->GetTeam() == HORDE)
                            {
                                me->CastSpell(caster, S_TORCH_TOSSING_TRAINING_SUCCESS_OK_H, true);
                                ((Player*)caster)->AreaExploredOrEventHappens(Q_MORE_TORCH_TOSSING_H);
                            }
                        }
                    }
                }
                else
                    me->CastSpell(caster, S_TORCHES_CAUGHT, true);
                TeleportTimer = 1000;
            }
        }
    }

    void DoChangeBrazier()
    {
        uint32 random = rand()%5;
        if(me->GetZoneId() == Z_DARNASSUS)
            me->NearTeleportTo(DarnassusLocations[random][0], DarnassusLocations[random][1], DarnassusLocations[random][2], DarnassusLocations[random][3]);
        else if(me->GetZoneId() == Z_IRONFORGE)
            me->NearTeleportTo(IronforgeLocations[random][0], IronforgeLocations[random][1], IronforgeLocations[random][2], IronforgeLocations[random][3]);
        else if(me->GetZoneId() == Z_STORMWIND)
            me->NearTeleportTo(StormwindLocations[random][0], StormwindLocations[random][1], StormwindLocations[random][2], StormwindLocations[random][3]);
        else if(me->GetZoneId() == Z_ORGRIMMAR)
            me->NearTeleportTo(OrgrimmarLocations[random][0], OrgrimmarLocations[random][1], OrgrimmarLocations[random][2], OrgrimmarLocations[random][3]);
        else if(me->GetZoneId() == Z_THUNDER_BLUFF)
            me->NearTeleportTo(ThunderBluffLocations[random][0], ThunderBluffLocations[random][1], ThunderBluffLocations[random][2], ThunderBluffLocations[random][3]);
        else if(me->GetZoneId() == Z_UNDERCITY)
            me->NearTeleportTo(UndercityLocations[random][0], UndercityLocations[random][1], UndercityLocations[random][2], UndercityLocations[random][3]);
        else if(me->GetZoneId() == Z_SILVEMOON)
            me->NearTeleportTo(SilvermoonLocations[random][0], SilvermoonLocations[random][1], SilvermoonLocations[random][2], SilvermoonLocations[random][3]);
        else if(me->GetZoneId() == Z_EXODAR)
            me->NearTeleportTo(ExodarLocations[random][0], ExodarLocations[random][1], ExodarLocations[random][2], ExodarLocations[random][3]);
    }

    void UpdateAI(const uint32 diff)
    {
        if(TeleportTimer <= diff)
        {
            DoChangeBrazier();
            TeleportTimer = 5000;
        }
        else
            TeleportTimer -= diff;
    }
};

CreatureAI* GetAI_npc_torch_tossing_target(Creature *_Creature)
{
    return new npc_torch_tossing_targetAI(_Creature);
}

/*####
## Quest(A/H): Incense for the Summer Scorchlings/Incense for the Festival Scorchlings
## Alliance ID: 11964
## Horde ID: 11966
## Comment: Timers may be adjusted.
####*/

enum SummerScorchling
{
    Q_INCENSE_FOR_THE_SUMMER_SCORCHLINGS   = 11964,
    Q_INCENSE_FOR_THE_FESTIVAL_SCORCHLINGS = 11966,
    S_GIVE_SUMMER_SCORCHLING_INCENSE       = 47107,
    EVENT_11964_1                          = 1,
    EVENT_11964_2                          = 2,
    EVENT_11964_3                          = 3,
    EVENT_11964_4                          = 4,
    EVENT_11964_5                          = 5,
    EVENT_11964_6                          = 6,
    EVENT_11964_7                          = 7,
    EVENT_11964_8                          = 8,
    EVENT_11964_9                          = 9,
    EVENT_11964_10                         = 10
    
};

struct npc_summer_scorchlingAI : public ScriptedAI
{
    npc_summer_scorchlingAI(Creature *c) : ScriptedAI(c) {}

    EventMap events;
    uint64 playerGUID;
    bool Started;

    void Reset()
    {
        playerGUID = 0;
        Started = false;
        me->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
        events.Reset();
    }
    
    void DoStartDialogue(Player* player)
    {
        if(!Started)
        {
            playerGUID = player->GetGUID();
            events.ScheduleEvent(EVENT_11964_1, 500);
            Started = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_11964_1:
                {
                    me->Say(-1200591, LANG_UNIVERSAL, playerGUID);
                    events.ScheduleEvent(EVENT_11964_2, 3000);
                    break;
                }
                case EVENT_11964_2:
                {
                    me->TextEmote(-1200592, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,2.0f);
                    events.ScheduleEvent(EVENT_11964_3, 1000);
                    break;
                }
                case EVENT_11964_3:
                {
                    me->Say(-1200593, LANG_UNIVERSAL, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,3.0f);
                    events.ScheduleEvent(EVENT_11964_4, 2000);
                    break;
                }
                case EVENT_11964_4:
                {
                    me->Say(-1200594, LANG_UNIVERSAL, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,4.0f);
                    events.ScheduleEvent(EVENT_11964_5, 3000);
                    break;
                }
                case EVENT_11964_5:
                {
                    me->Say(-1200595, LANG_UNIVERSAL, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,5.0f);
                    events.ScheduleEvent(EVENT_11964_6, 3000);
                    break;
                }
                case EVENT_11964_6:
                {
                    me->TextEmote(-1200596, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,6.0f);
                    events.ScheduleEvent(EVENT_11964_7, 3000);
                    break;
                }
                case EVENT_11964_7:
                {
                    me->Say(-1200597, LANG_UNIVERSAL, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,7.0f);
                    events.ScheduleEvent(EVENT_11964_8, 4000);
                    break;
                }
                case EVENT_11964_8:
                {
                    me->Say(-1200598, LANG_UNIVERSAL, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,8.0f);
                    events.ScheduleEvent(EVENT_11964_9, 2000);
                    break;
                }
                case EVENT_11964_9:
                {
                    me->TextEmote(-1200599, 0);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
                    events.ScheduleEvent(EVENT_11964_10, 1000);
                    break;
                }
                case EVENT_11964_10:
                {
                    me->Say(-1200600, LANG_UNIVERSAL, 0);
                    Reset();
                    break;
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_summer_scorchling(Creature *_Creature)
{
    return new npc_summer_scorchlingAI(_Creature);
}

bool QuestComplete_npc_summer_scorchling(Player *player, Creature *creature, const Quest *quest)
{
    if((quest->GetQuestId() == Q_INCENSE_FOR_THE_SUMMER_SCORCHLINGS) || (quest->GetQuestId() == Q_INCENSE_FOR_THE_FESTIVAL_SCORCHLINGS))
    {
        creature->CastSpell(player, S_GIVE_SUMMER_SCORCHLING_INCENSE, false);
        ((npc_summer_scorchlingAI*)creature->AI())->DoStartDialogue(player);
    }

    return true;
}

/*####
## Quest: Torch Catching & More Torch Catching
## Alliance ID: 11657, 11924
## Horde ID: 11923, 11925
## Comment:
####*/

enum qTorchCatching
{
    Q_TORCH_CATCHING_A      = 11657,
    Q_TORCH_CATCHING_H      = 11923,
    Q_MORE_TORCH_CATCHING_A = 11924,
    Q_MORE_TORCH_CATCHING_H = 11925,

    S_FLING_TORCH           = 46747, // This is used by item
    S_TORCHES_CAUGHT_2      = 45693,
    S_JUGGLE_TORCH          = 45792,
    S_TORCH_TOSS_SHADOW     = 46105,
    S_TORCH_CATCHING_SUCCS  = 46654,

    N_TORCH_CATCHING_BUNNY  = 26188,
    I_UNLIT_TORCHES         = 34833
    
};

struct npc_torch_catching_bunnyAI : public ScriptedAI
{
    npc_torch_catching_bunnyAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == S_TORCH_TOSS_SHADOW)
        {
            if(me->GetDistance2d(caster) <= 3)
            {
                if(((Player*)caster)->GetQuestStatus(Q_TORCH_CATCHING_A) == QUEST_STATUS_INCOMPLETE || ((Player*)caster)->GetQuestStatus(Q_TORCH_CATCHING_H) == QUEST_STATUS_INCOMPLETE)
                {
                    if(caster->HasAura(S_TORCHES_CAUGHT_2))
                    {
                        if(Aura* aura = caster->GetAura(S_TORCHES_CAUGHT_2, 1))
                        {
                            if(!aura || aura->GetStackAmount() <= 2)
                            {
                                me->CastSpell(caster, S_TORCHES_CAUGHT_2, true);
                                float x,y,z;
                                caster->GetRandomPoint(caster->GetPositionX(),caster->GetPositionY(),caster->GetPositionZ(),10.0f,x,y,z);
                                if(Creature* bunny = caster->SummonCreature(N_TORCH_CATCHING_BUNNY, x, y, z, caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7000))
                                {
                                    caster->CastSpell(bunny, S_JUGGLE_TORCH, true);
                                    caster->CastSpell(bunny, S_TORCH_TOSS_SHADOW, true);
                                }
                                me->DisappearAndDie();
                            }
                            else
                            {
                                caster->RemoveAurasDueToSpell(S_TORCHES_CAUGHT_2);
                                caster->CastSpell(caster, S_TORCH_CATCHING_SUCCS, true);
                                if(((Player*)caster)->GetTeam() == ALLIANCE)
                                    ((Player*)caster)->AreaExploredOrEventHappens(Q_TORCH_CATCHING_A);
                                else if(((Player*)caster)->GetTeam() == HORDE)
                                    ((Player*)caster)->AreaExploredOrEventHappens(Q_TORCH_CATCHING_H);
                            }
                        }
                    }
                    else
                    {
                        me->CastSpell(caster, S_TORCHES_CAUGHT_2, true);
                        float x,y,z;
                        caster->GetRandomPoint(caster->GetPositionX(),caster->GetPositionY(),caster->GetPositionZ(),10.0f,x,y,z);
                        if(Creature* bunny = caster->SummonCreature(N_TORCH_CATCHING_BUNNY, x, y, z, caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7000))
                        {
                            caster->CastSpell(bunny, S_JUGGLE_TORCH, true);
                            caster->CastSpell(bunny, S_TORCH_TOSS_SHADOW, true);
                        }
                    }
                }
                else if(((Player*)caster)->GetQuestStatus(Q_MORE_TORCH_CATCHING_A) == QUEST_STATUS_INCOMPLETE || ((Player*)caster)->GetQuestStatus(Q_MORE_TORCH_CATCHING_H) == QUEST_STATUS_INCOMPLETE)
                {
                    if(caster->HasAura(S_TORCHES_CAUGHT_2))
                    {
                        if(Aura* aura = caster->GetAura(S_TORCHES_CAUGHT_2, 1))
                        {
                            if(!aura || aura->GetStackAmount() <= 8)
                            {
                                me->CastSpell(caster, S_TORCHES_CAUGHT_2, true);
                                float x,y,z;
                                caster->GetRandomPoint(caster->GetPositionX()+4,caster->GetPositionY()+4,caster->GetPositionZ(),10.0f,x,y,z);
                                if(Creature* bunny = caster->SummonCreature(N_TORCH_CATCHING_BUNNY, x, y, z, caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7000))
                                {
                                    caster->CastSpell(bunny, S_JUGGLE_TORCH, true);
                                    caster->CastSpell(bunny, S_TORCH_TOSS_SHADOW, true);
                                }
                                me->DisappearAndDie();
                            }
                            else
                            {
                                caster->RemoveAurasDueToSpell(S_TORCHES_CAUGHT_2);
                                caster->CastSpell(caster, S_TORCH_CATCHING_SUCCS, true);
                                if(((Player*)caster)->GetTeam() == ALLIANCE)
                                    ((Player*)caster)->AreaExploredOrEventHappens(Q_MORE_TORCH_CATCHING_A);
                                else if(((Player*)caster)->GetTeam() == HORDE)
                                    ((Player*)caster)->AreaExploredOrEventHappens(Q_MORE_TORCH_CATCHING_H);
                            }
                        }
                    }
                    else
                    {
                        me->CastSpell(caster, S_TORCHES_CAUGHT_2, true);
                        float x,y,z;
                        caster->GetRandomPoint(caster->GetPositionX()+4,caster->GetPositionY()+4,caster->GetPositionZ(),10.0f,x,y,z);
                        if(Creature* bunny = caster->SummonCreature(N_TORCH_CATCHING_BUNNY, x, y, z, caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7000))
                        {
                            caster->CastSpell(bunny, S_JUGGLE_TORCH, true);
                            caster->CastSpell(bunny, S_TORCH_TOSS_SHADOW, true);
                        }
                    }
                }
            }
            else
            {
                if(caster->HasAura(S_TORCHES_CAUGHT_2))
                    caster->RemoveAurasDueToSpell(S_TORCHES_CAUGHT_2);
            }
        }
    }
};

CreatureAI* GetAI_npc_torch_catching_bunny(Creature *_Creature)
{
    return new npc_torch_catching_bunnyAI(_Creature);
}
        
bool ItemUse_item_unlit_torches(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if ((player->GetQuestStatus(Q_TORCH_CATCHING_A) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(Q_TORCH_CATCHING_H) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(Q_MORE_TORCH_CATCHING_A) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(Q_MORE_TORCH_CATCHING_H) == QUEST_STATUS_INCOMPLETE))
    {
        GameObject* pGo = GetClosestGameObjectWithEntry(player, 300068, 5.0f);

        if(!pGo)
            return true;

        if(player->HasUnitState(UNIT_STAT_CASTING))
            return true;

        float x,y,z;
        player->GetRandomPoint(player->GetPositionX()+4,player->GetPositionY()+4,player->GetPositionZ(),15.0f,x,y,z);
        if(Creature* bunny = player->SummonCreature(N_TORCH_CATCHING_BUNNY, x, y, z, player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7000))
        {
            player->CastSpell(bunny, S_JUGGLE_TORCH, true);
            player->CastSpell(bunny, S_TORCH_TOSS_SHADOW, true);
        }
        return false;
    }
    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_ERROR);                      // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target

    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

/*######
## NPC: Dancing Flames
## ID: 25305
## Item ID: 34686
## Comment:
######*/

#define SPELL_BRAZIER       45423
#define SPELL_SEDUCTION     47057
#define SPELL_FIERY_AURA    45427

#define SPELL_BRAZIER       45423
#define SPELL_SEDUCTION     47057
#define SPELL_FIERY_AURA    45427

struct npc_dancing_flamesAI : public ScriptedAI
{
    npc_dancing_flamesAI(Creature *c) : ScriptedAI(c) {}

    bool active;
    Timer_UnCheked CanIteractTimer;

    void Reset()
    {
        active = true;
        CanIteractTimer.Reset(3500);
        DoCast(me,SPELL_BRAZIER,true);
        DoCast(me,SPELL_FIERY_AURA,false);
        float x, y, z;
        me->GetPosition(x,y,z);
        me->Relocate(x,y,z + 0.94f);
        me->SetLevitate(true);
        me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!active && CanIteractTimer.Expired(diff))
        {
            active = true;
            CanIteractTimer = 3500;
            me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
        }
    }
};

CreatureAI* GetAI_npc_dancing_flames(Creature *_Creature)
{
    return new npc_dancing_flamesAI(_Creature);
}

bool ReceiveEmote_npc_dancing_flames( Player *player, Creature *flame, uint32 emote )
{
    if ( ((npc_dancing_flamesAI*)flame->AI())->active &&
            flame->IsWithinLOS(player->GetPositionX(),player->GetPositionY(),player->GetPositionZ()) && flame->IsWithinDistInMap(player,30.0f))
    {
        flame->SetInFront(player);
        ((npc_dancing_flamesAI*)flame->AI())->active = false;

        switch(emote)
        {
        case TEXTEMOTE_KISS:
            flame->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
            break;
        case TEXTEMOTE_WAVE:
            flame->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            break;
        case TEXTEMOTE_BOW:
            flame->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
            break;
        case TEXTEMOTE_JOKE:
            flame->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
            break;
        case TEXTEMOTE_DANCE:
        {
            if (!player->HasAura(SPELL_SEDUCTION,0))
                flame->CastSpell(player,SPELL_SEDUCTION,true);
        }
        break;
        }
    }
    return true;
}

/*######
## NPC: Fire Eater, Master Fire Eater, Flame Eater, Master Flame Eater
## ID:  25962,      25975,             25994,       26113
## Comment:
######*/

#define SPELL_MID_FLAME_BREATH  46332

struct npc_flame_or_fire_eaterAI : public ScriptedAI
{
    npc_flame_or_fire_eaterAI(Creature *c) : ScriptedAI(c) {}

    Timer FlameBreathTimer;

    void Reset()
    {
        FlameBreathTimer.Reset(3000);
        DoCast(me, SPELL_MID_FLAME_BREATH, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (FlameBreathTimer.Expired(diff))
        {
            DoCast(me, SPELL_MID_FLAME_BREATH, true);
            FlameBreathTimer = urand(15000, 25000);
        }
    }
};

CreatureAI* GetAI_npc_flame_or_fire_eater(Creature *_Creature)
{
    return new npc_flame_or_fire_eaterAI(_Creature);
}

/*######
## GameObject: Ribbon Pole
## ID: 181605
## Comment:
######*/

enum RibbonPoleSpellList
{
    SPELL_RP_CHANNEL_1                  = 29172, // wtf
    SPELL_RP_CHANNEL_2                  = 29531, // wtf
    SPELL_RP_RITUAL_EFFECT              = 29173, // wtf
    SPELL_RP_XP_BUFF                    = 29175,
    SPELL_RP_SUMMON_TRIGGER             = 29708, // we wouldn't use this spell
    SPELL_RP_CHANNEL_VIS_1              = 29705, // Casted on channeler on use
    SPELL_RP_CHANNEL_VIS_2              = 29726, // Casted on channeler on use
    SPELL_RP_CHANNEL_VIS_3              = 29727, // Casted on channeler on use
    SPELL_RP_WHIRLWIND                  = 45406, // Casted on channeler on use
    SPELL_RP_CHECK_AURA_1               = 45390, // buff, periodic
    SPELL_RP_CHECK_AURA_2               = 45405, // instant
    SPELL_RP_FIREWORK_AND_FLAME         = 46829, // casted on 1 channeler
    SPELL_RP_FLAME_RING                 = 46842, // caster on 1 channeler
    SPELL_RP_FIREWORK_AND_FLAME_LONG    = 46830, // casted on 2 channelers
    SPELL_RP_FIREWORK_FLAME_RING        = 46835, // casted on 2 channelers
    SPELL_RP_GROUND_FLOWER_FLAME_RING   = 46971, // casted on 3-5 channelers
    SPELL_RP_FIRE_SPIRAL_SUMMON         = 45422, // NOT USED BUT should be casted on 6+ channelers - white fire under caster + spawn 25303 creature
    SPELL_RP_FIRE_SPIRAL                = 45421, // casted on 6+ channelers - should be casted on 17066 by 25303
    SPELL_RP_BIG_FLAME_DANCER           = 46827, // casted on 6+ channelers - summon Big Dancing Flames near 25303 and flame patch visual
    SPELL_RP_PLAY_MUSIC                 = 48652,
    SPELL_RP_PLAY_MUSIC_2               = 46896,    

    NPC_RIBBON_POLE_DEBUG_TARGET        = 17066,
    NPC_RIBBON_POLE_FIRE_SPIRAL_BUNNY   = 25303
};

bool GOUse_go_ribbon_pole(Player *player, GameObject* go)
{
    if (Creature* RibbonPoleTarget = GetClosestCreatureWithEntry(go, NPC_RIBBON_POLE_DEBUG_TARGET, 10))
    {
        player->CastSpell(RibbonPoleTarget, RAND(SPELL_RP_CHANNEL_VIS_1, SPELL_RP_CHANNEL_VIS_2, SPELL_RP_CHANNEL_VIS_3), true);
        if (!player->HasAura(SPELL_RP_WHIRLWIND))
            player->CastSpell(player, SPELL_RP_WHIRLWIND, true);
    }

    return true;
}

/*######
## NPC: Ribbon Pole Debug Target
## ID:  17066
## Comment:
######*/

struct ribbon_pole_debug_targetAI : public ScriptedAI
{
    ribbon_pole_debug_targetAI(Creature *c) : ScriptedAI(c) {}

    uint32 ChannelersCount;
    Timer FireWorkPatchTimer;
    Timer FlameRingTimer;
    Timer GroundFlowerTimer;
    Timer LavaSpiralTimer;

    void Reset()
    {
        ChannelersCount = 0;
        FireWorkPatchTimer.Reset(0);
        FlameRingTimer.Reset(0);
        GroundFlowerTimer.Reset(0);
        LavaSpiralTimer.Reset(0);
    }

    void SpellHit(Unit* caster, const SpellEntry* SpellEntry) 
    {
        if (caster && ((SpellEntry->Id == SPELL_RP_CHANNEL_VIS_1) || SpellEntry->Id == SPELL_RP_CHANNEL_VIS_2) || (SpellEntry->Id == SPELL_RP_CHANNEL_VIS_3))
        {
            ChannelersCount++;
            switch (ChannelersCount)
            {
                case 0: break;
                case 1:
                {
                    FireWorkPatchTimer = 1;
                    FlameRingTimer = 0;
                    GroundFlowerTimer = 0;
                    LavaSpiralTimer = 0;
                    break;
                }
                case 3:
                case 4:
                case 5:
                {
                    GroundFlowerTimer = 1;
                    FireWorkPatchTimer = 0;
                    FlameRingTimer = 0;
                    LavaSpiralTimer = 0;
                    break;
                }
                case 6:
                {
                    LavaSpiralTimer = 1;
                    GroundFlowerTimer = 0;
                    FireWorkPatchTimer = 0;
                    FlameRingTimer = 0;
                    break;
                }
                default:
                {
                    ChannelersCount = 6;
                    break;
                }
            }
        }
    }

    void OnAuraRemove(Aura* Aura, bool RemoveFromStack)
    {
        if(RemoveFromStack)
        {
            if((Aura->GetId() == SPELL_RP_CHANNEL_VIS_1) || (Aura->GetId() == SPELL_RP_CHANNEL_VIS_2) || (Aura->GetId() == SPELL_RP_CHANNEL_VIS_3))
                ChannelersCount--;
        }
    }
    void UpdateAI(const uint32 diff)
    {
        switch (ChannelersCount)
        {
            case 1:
            {
                if (FireWorkPatchTimer.Expired(diff))
                {
                    if(Creature *trigger = me->SummonTrigger(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()-6, 0, 5000))
                    {
                        trigger->CastSpell(trigger, SPELL_RP_FIREWORK_AND_FLAME_LONG, true);
                        FlameRingTimer = 5000;
                        FireWorkPatchTimer = 0;
                    }
                }
                if (FlameRingTimer.Expired(diff))
                {
                    if(Creature *trigger = me->SummonTrigger(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()-6, 0, 5000))
                    {
                        trigger->CastSpell(trigger, SPELL_RP_FLAME_RING, true);
                        FlameRingTimer = 0;
                        FireWorkPatchTimer = 5000;
                    }
                }
                break;
            }
            case 2:
            {
                if (FireWorkPatchTimer.Expired(diff))
                {
                    if(Creature *trigger = me->SummonTrigger(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()-6, 0, 5000))
                    {
                        trigger->CastSpell(trigger, SPELL_RP_FIREWORK_AND_FLAME, true);
                        FlameRingTimer = 5000;
                        FireWorkPatchTimer = 0;
                    }
                }
                if (FlameRingTimer.Expired(diff))
                {
                    if(Creature *trigger = me->SummonTrigger(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()-6, 0, 5000))
                    {
                        trigger->CastSpell(trigger, SPELL_RP_FIREWORK_FLAME_RING, true);
                        FlameRingTimer = 0;
                        FireWorkPatchTimer = 5000;
                    }
                }
                break;
            }
            case 3:
            case 4:
            case 5:
            {
                if (GroundFlowerTimer.Expired(diff))
                {
                    if(Creature *trigger = me->SummonTrigger(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()-6, 0, 5000))
                    {
                        trigger->CastSpell(trigger, SPELL_RP_GROUND_FLOWER_FLAME_RING, true);
                        GroundFlowerTimer = 5000;
                    }
                }
                break;
            }
            case 6:
            {
                if (LavaSpiralTimer.Expired(diff))
                {
                    if(Creature *trigger = me->SummonTrigger(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()-6, 0, 8000))
                    {
                        trigger->CastSpell(trigger, SPELL_RP_FLAME_RING, true);
                        float x,y,z;
                        trigger->GetRandomPoint(trigger->GetPositionX()+8,trigger->GetPositionY()+8,trigger->GetPositionZ(),10.0f,x,y,z);
                        if(Creature* bunny = trigger->SummonCreature(NPC_RIBBON_POLE_FIRE_SPIRAL_BUNNY, x, y, z, trigger->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 5000))
                        {
                            bunny->CastSpell(bunny, SPELL_RP_BIG_FLAME_DANCER, true);
                            bunny->CastSpell(me, SPELL_RP_FIRE_SPIRAL, true);
                            bunny->GetMotionMaster()->MoveRandomAroundPoint(bunny->GetPositionX(), bunny->GetPositionY(), bunny->GetPositionZ(), 5.0f);
                        }
                        LavaSpiralTimer = 5000;
                    }
                }
                break;
            }
            default: break;
        }
    }
};

CreatureAI* GetAI_ribbon_pole_debug_target(Creature *_Creature)
{
    return new ribbon_pole_debug_targetAI(_Creature);
}
void AddSC_midsummer()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_torch_tossing_target";
    newscript->GetAI = &GetAI_npc_torch_tossing_target;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_summer_scorchling";
    newscript->GetAI = &GetAI_npc_summer_scorchling;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_summer_scorchling;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_torch_catching_bunny";
    newscript->GetAI = &GetAI_npc_torch_catching_bunny;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_unlit_torches";
    newscript->pItemUse = &ItemUse_item_unlit_torches;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dancing_flames";
    newscript->GetAI = &GetAI_npc_dancing_flames;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_dancing_flames;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_flame_or_fire_eater";
    newscript->GetAI = &GetAI_npc_flame_or_fire_eater;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_ribbon_pole";
    newscript->pGOUse = &GOUse_go_ribbon_pole;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ribbon_pole_debug_target";
    newscript->GetAI = &GetAI_ribbon_pole_debug_target;
    newscript->RegisterSelf();
}