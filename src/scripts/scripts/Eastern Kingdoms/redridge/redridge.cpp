#include "precompiled.h"
#include "escort_ai.h"

#define QUEST_MISSING_IN_ACTION 219

struct npc_corporal_keeshanAI : public npc_escortAI
{
    npc_corporal_keeshanAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    Timer MockingBlowTimer;
    Timer ShieldBashTimer;
    Timer WaitTimer;
    uint8 Phase;

    void Reset()
    {
        MockingBlowTimer.Reset(5000);
        ShieldBashTimer.Reset(8000);
        WaitTimer.Reset(0);
        Phase = 0;
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;
        
        switch(i)
        {
            case 26:
                SetEscortPaused(true);
                WaitTimer = 15000;
                Phase = 1;
                me->SetStandState(UNIT_STAND_STATE_SIT);
                DoScriptText(-1230043, me, 0);
                break;
            case 53:
                DoScriptText(-1230044, me, 0);
                pPlayer->GroupEventHappens(QUEST_MISSING_IN_ACTION, me);
                Phase = 2;
                WaitTimer = 2000;
                SetEscortPaused(true);
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if(Phase)
        {
            if(WaitTimer.Expired(diff))
            {
                switch(Phase)
                {
                    case 1:
                        DoScriptText(-1230045, me, 0);
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        SetEscortPaused(false);
                        me->SetWalk(false);
                        Phase = 0;
                        WaitTimer = 0;
                        break;
                    case 2:
                        DoScriptText(-1230046, me, 0);
                        SetEscortPaused(false);
                        Phase = 0;
                        WaitTimer = 0;
                        break;
                }
            }
        }

        if(!UpdateVictim())
            return;

        if(MockingBlowTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 21008, false);
            MockingBlowTimer = 5000;
        }

        if(ShieldBashTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 11972, false);
            ShieldBashTimer = 8000;
        }

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_corporal_keeshan(Creature* pCreature)
{
    npc_corporal_keeshanAI* thisAI = new npc_corporal_keeshanAI(pCreature);
    thisAI->AddWaypoint(1,  -8769.591797, -2185.733643, 141.974564);
    thisAI->AddWaypoint(2,  -8776.540039, -2193.782959, 140.960159);
    thisAI->AddWaypoint(3,  -8783.289063, -2194.818604, 140.461731);
    thisAI->AddWaypoint(4,  -8792.520508, -2188.802002, 142.077728);
    thisAI->AddWaypoint(5,  -8807.547852, -2186.100830, 141.504135);
    thisAI->AddWaypoint(6,  -8818,        -2184.8,      139.153);
    thisAI->AddWaypoint(7,  -8825.805664, -2188.840576, 138.458832);
    thisAI->AddWaypoint(8,  -8827.522461, -2199.805664, 139.621933);
    thisAI->AddWaypoint(9,  -8821.140625, -2212.642334, 143.126419);
    thisAI->AddWaypoint(10,  -8809.175781, -2230.456299, 143.438431);
    thisAI->AddWaypoint(11,  -8797.040039, -2240.718262, 146.548203);
    thisAI->AddWaypoint(12,  -8795.242188, -2251.809814, 146.808044);
    thisAI->AddWaypoint(13,  -8780.159180, -2258.615967, 148.553772);
    thisAI->AddWaypoint(14,  -8762.650391, -2259.559326, 151.144241);
    thisAI->AddWaypoint(15,  -8754.357422, -2253.735352, 152.243073);
    thisAI->AddWaypoint(16,  -8741.869141, -2250.997070, 154.485718);
    thisAI->AddWaypoint(17,  -8733.218750, -2251.010742, 154.360031);
    thisAI->AddWaypoint(18,  -8717.474609, -2245.044678, 154.68614);
    thisAI->AddWaypoint(19,  -8712.240234, -2246.826172, 154.709473);
    thisAI->AddWaypoint(20,  -8693.840820, -2240.410889, 152.909714);
    thisAI->AddWaypoint(21,  -8681.818359, -2245.332764, 155.517838);
    thisAI->AddWaypoint(22,  -8669.86,     -2252.77,     154.854);
    thisAI->AddWaypoint(23,  -8670.56,     -2264.69,     156.978);
    thisAI->AddWaypoint(24,  -8676.557617, -2269.204346, 155.411316);
    thisAI->AddWaypoint(25,  -8673.340820, -2288.650146, 157.054123);
    thisAI->AddWaypoint(26,  -8677.760742, -2302.563965, 155.916580);
    thisAI->AddWaypoint(27,  -8682.462891, -2321.688232, 155.916946);
    thisAI->AddWaypoint(28,  -8690.402344, -2331.779297, 155.970505);
    thisAI->AddWaypoint(29,  -8715.1,      -2353.95,     156.188);
    thisAI->AddWaypoint(30,  -8748.042969, -2370.701904, 157.988342);
    thisAI->AddWaypoint(31,  -8780.900391, -2421.370361, 156.108871);
    thisAI->AddWaypoint(32,  -8792.009766, -2453.379883, 142.746002);
    thisAI->AddWaypoint(33,  -8804.780273, -2472.429932, 134.192001);
    thisAI->AddWaypoint(34,  -8841.348633, -2503.626221, 132.276138);
    thisAI->AddWaypoint(35,  -8867.565430, -2529.892822, 134.738586);
    thisAI->AddWaypoint(36,  -8870.67,     -2542.08,     131.044);
    thisAI->AddWaypoint(37,  -8922.05,     -2585.31,     132.446);
    thisAI->AddWaypoint(38,  -8949.08,     -2596.87,     132.537);
    thisAI->AddWaypoint(39,  -8993.460938, -2604.042725, 130.756210);
    thisAI->AddWaypoint(40,  -9006.709961, -2598.469971, 127.966003);
    thisAI->AddWaypoint(41,  -9038.96,     -2572.71,     124.748);
    thisAI->AddWaypoint(42,  -9046.92,     -2560.64,     124.447);
    thisAI->AddWaypoint(43,  -9066.693359, -2546.633301, 123.110138);
    thisAI->AddWaypoint(44,  -9077.54,     -2541.67,     121.17);
    thisAI->AddWaypoint(45,  -9125.320313, -2490.059326, 116.057274);
    thisAI->AddWaypoint(46,  -9145.063477, -2442.239990, 108.231689);
    thisAI->AddWaypoint(47,  -9158.197266, -2425.363281, 105.500038);
    thisAI->AddWaypoint(48,  -9151.922852, -2393.671631, 100.856010);
    thisAI->AddWaypoint(49,  -9165.193359, -2376.031738, 94.821518);
    thisAI->AddWaypoint(50,  -9187.099609, -2360.520020, 89.923103);
    thisAI->AddWaypoint(51,  -9235.443359, -2305.239014, 77.925316);
    thisAI->AddWaypoint(52,  -9264.73,     -2292.92,     70.0089);
    thisAI->AddWaypoint(53,  -9277.468750, -2296.188721, 68.089630);
    thisAI->AddWaypoint(54,  -9277.468750, -2296.188721, 68.089630);
    return (CreatureAI*)thisAI;
}

bool QuestAccept_npc_corporal_keeshan(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_MISSING_IN_ACTION)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_corporal_keeshanAI, pCreature->AI()))
        {
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest, true);
            pCreature->setFaction(FACTION_ESCORT_A_NEUTRAL_ACTIVE);
            pCreature->SetAggroRange(10);
            DoScriptText(-1230047, pCreature, 0);
        }
    }
    return true;
}

void AddSC_redridge()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_corporal_keeshan";
    newscript->GetAI = &GetAI_npc_corporal_keeshan;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_corporal_keeshan;
    newscript->RegisterSelf();
}
