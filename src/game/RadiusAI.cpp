#include "RadiusAI.h"
#include "ObjectAccessor.h"
#include "CreatureAIImpl.h"

enum SafePositions
{
    //SAFE_WSG_ALLIANCE_BUILDING = 690100, // flag room & from resp
    //SAFE_WSG_ALLIANCE_RESP_CLIMB = 690101,
    //SAFE_WSG_ALLIANCE_RIGHT_CLIMB = 690102,
    SAFE_WSG_MIDDLE_STUMP_1 = 690100, // radius 15 .go xyz 1224.867676 1630.112183 326.357849
    SAFE_WSG_MIDDLE_STUMP_2 = 690101, // 10 .go xyz 1269.643677 1597.056030 314.937561
};

Position GetSafePosition(SafePositions id)
{
    Position pos;
    
    switch (id)
    {
    //case SAFE_WSG_ALLIANCE_BUILDING:
    //    return { 1510.336670f, 1516.466431f, 354.554962f };
    //case SAFE_WSG_ALLIANCE_RESP_CLIMB:
    //    return { 1432.624268f, 1634.779907f, 349.162750f };
    //case SAFE_WSG_ALLIANCE_RIGHT_CLIMB:
    //    return { 1526.548706f, 1414.713257f, 372.655029f };
    case SAFE_WSG_MIDDLE_STUMP_1:
        return { 1210.809448f, 1616.365112f, 321.336578f };
    case SAFE_WSG_MIDDLE_STUMP_2:
        return { 1270.449585f, 1593.425415f, 318.409363f };
    //case SAFE_WSG_MIDDLE_HORDE_CLIMB:
    //    return { 1185.934326f, 1285.717163f, 316.719177f };
    default:
        throw std::invalid_argument("");
    }
}

void
RadiusAI::MoveInLineOfSight(Unit* who)
{
    if (who->GetTypeId() != TYPEID_PLAYER)
        return;

    // armor is visibility range
    uint32 range = me->GetArmor();
    
    if (me->GetDistance(who) > range)
        return;

    try 
    {
        Position pos = GetSafePosition(SafePositions(me->GetEntry()));
        who->NearTeleportTo(pos.x, pos.y, pos.z, who->GetOrientation());
        sLog.outLog(LOG_SPECIAL, "RadiusAI NPC %u just teleported %s", me->GetEntry(), who->GetName());
    }
    catch (const std::invalid_argument) 
    {
        sLog.outLog(LOG_CRITICAL, "RadiusAI NPC %u have problems", me->GetEntry());
    }
}