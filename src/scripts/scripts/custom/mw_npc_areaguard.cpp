// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// Not owned by DeathSide, Trentone :D, cause of that name doesn't contain DeathSide
#include "precompiled.h"

/*******************************************************
 * npc_areaguard
 *******************************************************/

struct npc_areaguardAI : public Scripted_NoMovementAI
{
    npc_areaguardAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        pCreature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        pCreature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_NORMAL, true);
        pCreature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);

        isExist = false;

        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT `guard_type`, `trigger_distance`, `req_value`,`tele_map`,`tele_x`,`tele_y`,`tele_z`,`tele_o` FROM `npc_areaguard` WHERE `creature_entry` = %i LIMIT 1",me->GetEntry());
        if (result)
        {
            Field *fields = result->Fetch();
            guardType = fields[0].GetUInt32();
            triggerDist = fields[1].GetFloat();
            reqValue = fields[2].GetUInt32();
            teleMap = fields[3].GetUInt32();
            teleX = fields[4].GetFloat();
            teleY = fields[5].GetFloat();
            teleZ = fields[6].GetFloat();
            teleO = fields[7].GetFloat();
            isExist = true;
        }
    }

    uint32 guardType;
    uint32 triggerDist;
    uint32 reqValue;
    uint32 teleMap;
    float teleX,teleY,teleZ,teleO;
    bool isExist;

    void Reset(){}

    void EnterCombat(Unit* /*pWho*/){}

    void AttackStart(Unit* /*pWho*/){}

    void MoveInLineOfSight(Unit *pWho)
    {
        if (!pWho || !pWho->IsInWorld())
            return;

        // Return if Distance is greater than maximum (65 ft) - saves data reads
        if (!me->IsWithinDist(pWho, 65.0f, true))
            return;

        if (!isExist)
            return;

        // Return if distance is greater than the trigger_distance
        if (!me->IsWithinDist(pWho, triggerDist, true))
            return;

        Player *pPlayer = pWho->GetCharmerOrOwnerPlayerOrPlayerItself();

        // Return if player has GM flag on or is in process of teleport
        if (!pPlayer || pPlayer->isGameMaster() || pPlayer->IsBeingTeleported())
            return;

        switch (guardType)
        {
            case 0:
                // Action on all players without GM flag on
                pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                break;
            case 1:
                {
                // Action based on Team
                if (reqValue == 1) // Alliance Required 
                    if (pPlayer->GetTeam() != 469) // Player is Horde
                        pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                if (reqValue == 2) // Horde Required
                    if (pPlayer->GetTeam() != 67) // Player is Alliance
                        pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                }
                break;
            case 2:
                {
                // Action based on GM Level
                if (pPlayer->GetSession()->GetPermissions() < reqValue)
                    pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                }
                break;
            case 3:
                {
                // Action based on Player Level
                if (pPlayer->GetLevel() < reqValue)
                    pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                }
                break;
            case 4:
                {
                // Action based on Guild ID
                if (pPlayer->GetGuildId() != reqValue)
                    pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                }
                break;
            case 5:
                {
                // Action based on Player GUID
                if (pPlayer->GetGUID() != reqValue)
                    pPlayer->TeleportTo(teleMap,teleX,teleY,teleZ,teleO);
                }
                break;
        }
 
        float x,y,z,o;
        me->GetHomePosition(x,y,z,o);
        me->SetOrientation(o);
        return;
    }

    void UpdateAI(const uint32 /*diff*/){}
};

CreatureAI* GetAI_npc_areaguard(Creature* pCreature)
{
    return new npc_areaguardAI(pCreature);
}

void AddSC_npc_areaguard()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_areaguard";
    newscript->GetAI = &GetAI_npc_areaguard;
    newscript->RegisterSelf();
}