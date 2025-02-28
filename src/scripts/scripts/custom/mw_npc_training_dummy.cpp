// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "mw_npc_training_dummy.h"
#include "Language.h"

struct DeathSide_Training_DummyAI : public Scripted_NoMovementAI
{
    DeathSide_Training_DummyAI(Creature *c) : Scripted_NoMovementAI(c) {}

    uint32 FightTime;
    uint32 DistanceTimer;
    uint32 CombatTimer;
    uint32 HealCheckTimer;
    uint32 CurrentHP;
    uint32 HPCheckTimer;
    uint32 MaxCombatTime;
    uint32 DamageTicks;
    bool FightStarted;

    BattleTypeIds BattleType;
    bool IsPersonalBattle;
    std::string PlayerName;

    void Reset()
    {
        me->RestoreFaction();
        FightTime = 0;
        DamageTicks = 0;
        DistanceTimer = 1000;
        CombatTimer = 5000;
        
        HealCheckTimer = 15000;
        CurrentHP = me->GetHealth();
        HPCheckTimer = 1000;
        FightStarted = false;
        
        MaxCombatTime = 180000;

        if (sWorld.getConfig(CONFIG_IS_LOCAL))
        {
            if (me->GetEntry() == 555055)
                MaxCombatTime = 15000;
            else if (me->GetEntry() == 555056)
                MaxCombatTime = 30000;
            else
                MaxCombatTime = 0;
        }

        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        for (uint8 school = SPELL_SCHOOL_NORMAL; school <= SPELL_SCHOOL_ARCANE; school++)
        {
            me->SetResistance(SpellSchools(school), 0);
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, (1 << school), false);
        }

        // GENSENTODO
        me->SetResistance(SPELL_SCHOOL_NORMAL, 5000);

        me->SetLevel(70);
        me->RemoveAurasDueToSpell(54742);
        me->RemoveAurasDueToSpell(20064);
        me->RemoveAurasDueToSpell(10021);
        me->SetStat(STAT_STRENGTH, 0);
        IsPersonalBattle = true;
        BattleType = BTYPE_DD; // DD by default
        me->SetIgnoreVictimSelection(true);
        PlayerName = "Not_Assigned";
    }

    void EnterCombat(Unit *who)
    { 
        me->SetIgnoreVictimSelection(true);
    }

    void DamageTaken(Unit* /*pKiller*/, uint32 &/*damage*/)
    {
        if (!FightStarted)
            FightStarted = true;

        ++DamageTicks;
        CombatTimer = 5000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FightStarted || BattleType != BTYPE_DD)
            FightTime += diff;
  
        if (MaxCombatTime > 0 && FightTime > MaxCombatTime)
        {
            SayMeters(me->GetVictim(), 0);
            EnterEvadeMode();
            return;
        }

        if (DistanceTimer <= diff)
        {
            if (me->GetDistance(me->GetVictim()) > 50 || BattleType == BTYPE_HEAL && me->GetDistance(me->GetVictim()) > 15)
            {
                SayMeters(me->GetVictim(), 0);
                EnterEvadeMode();
                return;
            }
            DistanceTimer = 1000;
            if (BattleType == BTYPE_TANK)
            {
                if (me->GetVictim()->getPowerType() == POWER_RAGE)
                    me->GetVictim()->SetPower(POWER_RAGE, me->GetVictim()->GetMaxPower(POWER_RAGE));
                else if (me->GetVictim()->GetClass() == CLASS_PALADIN)
                    me->GetVictim()->SetPower(POWER_MANA, me->GetVictim()->GetMaxPower(POWER_MANA));
            }
        }
        else
            DistanceTimer -= diff;

        if (BattleType == BTYPE_DD || BattleType == BTYPE_TANK)
        {
            if (CombatTimer <= diff)
            {
                SayMeters(me->GetVictim(), 0);
                EnterEvadeMode();
            }
            else
                CombatTimer -= diff;
        }
        else
        {
            if (HPCheckTimer <= diff)
            {
                if (me->GetVictim()->GetHealth() > 1)
                {
                    if (me->GetVictim()->GetHealth() - 1 > 15)
                        me->SetHealth(me->GetHealth() + me->GetVictim()->GetHealth() - 1);
                    me->GetVictim()->SetHealth(1);
                }
                HPCheckTimer = 1000;
            }
            else
                HPCheckTimer -= diff;

            if (HealCheckTimer <= diff)
            {
                if (CurrentHP == me->GetHealth())
                {
                    SayMeters(me->GetVictim(), 15000);
                    EnterEvadeMode();
                }
                else
                    CurrentHP = me->GetHealth();
                HealCheckTimer = 15000;
            }
            else
                HealCheckTimer -= diff;
        }
    }

    void SayMeters (Unit* unit, uint32 time)
    {
        char Chr[512];
        char ChrIfPersonal[128];
        Player* pPlayer = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!pPlayer)
        {
            me->Say("Error! Cannot find player!", LANG_UNIVERSAL, 0);
            return;
        }

        // player can't select gossip if in duel
        if (pPlayer->duel)
            return;

        if (IsPersonalBattle)
            sprintf(ChrIfPersonal, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BATTLE_END_IF_PERSONAL), PlayerName.c_str());
        else
            ChrIfPersonal[0] = 0;
        float TimeInFight = ((float)FightTime-time)/1000;
        if (TimeInFight <= 0)
            TimeInFight = 1;

        switch (BattleType) // Here aggrolists/ dd lists and heal lists
        {
            case BTYPE_DD:
            {
                if (me->GetHealth() < 200000000)
                {
                    sprintf(Chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BATTLE_END_DD), ChrIfPersonal, 200000000 - me->GetHealth(),
                        uint32(round((200000000 - me->GetHealth()) / TimeInFight)), DamageTicks, uint32(round(TimeInFight)));
                    me->Say(Chr, LANG_UNIVERSAL, 0);
                }
                break;
            }
            case BTYPE_HEAL:
            {
                if (me->GetHealth() > 200000000)
                {
                    sprintf(Chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BATTLE_END_HEAL), ChrIfPersonal, me->GetHealth() - 200000000,
                        uint32(round((me->GetHealth() - 200000000) / TimeInFight)), uint32(round(TimeInFight)));
                    me->Say(Chr, LANG_UNIVERSAL, 0);
                }
                break;
            }
            case BTYPE_TANK:
            {
                if (unit && me->getThreatManager().getThreat(unit))
                {
                    sprintf(Chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BATTLE_END_TANK), ChrIfPersonal, uint32(me->getThreatManager().getThreat(unit)),
                        uint32(round(me->getThreatManager().getThreat(unit) / TimeInFight)), uint32(round(TimeInFight)));
                    me->Say(Chr, LANG_UNIVERSAL, 0);
                }
                break;
            }
            default:
                break;
        }
        pPlayer->restorePowers();
        pPlayer->RemoveArenaSpellCooldowns();
        pPlayer->RemoveAurasDueToSpell(54860);
        pPlayer->RemoveAurasDueToSpell(54861);
        pPlayer->RemoveAurasDueToSpell(54862);
    }
};

bool DeathSide_Training_Dummy_Hello(Player* pPlayer, Creature* pCreature)
{
    pPlayer->PlayerTalkClass->ClearMenus();
    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_CHANGE_OPTIONS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 17);
    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_RESET_OPTIONS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
    if (((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle)
        pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_OFF_PERSONAL_FIGHT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);
    else
        pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_ON_PERSONAL_FIGHT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAIN_AS_DD), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAIN_AS_TANK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAIN_AS_HEALER), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
    pPlayer->SEND_GOSSIP_MENU(16,pCreature->GetGUID());
    return true;
}

bool DeathSide_Training_Dummy_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    // player can't select gossip if in duel
    if (pPlayer->duel)
        return false;

    if (pCreature->IsInCombat())
        return false;

    if (pPlayer->IsPvP())
    {
        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_DISABLE_OFF_PVP_FLAG), pPlayer->GetGUID());
        return false;
    }
    
    pCreature->SetMaxHealth(1000000000);
    pCreature->SetHealth(200000000);

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            ((DeathSide_Training_DummyAI*)pCreature->AI())->CombatTimer = true;
        }
        case GOSSIP_ACTION_INFO_DEF+8:
        {
            for (uint8 school = SPELL_SCHOOL_NORMAL; school <= SPELL_SCHOOL_ARCANE; school++)
            {
                pCreature->SetResistance(SpellSchools(school), 0);
                pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, (1 << school), false);
            }
            pCreature->SetLevel(70);
            pCreature->RemoveAurasDueToSpell(54742);
            pCreature->RemoveAurasDueToSpell(20064);
            pCreature->RemoveAurasDueToSpell(10021);
            pCreature->SetStat(STAT_STRENGTH, 0);
            ((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle = true;
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_DEFAULT_OPTIONS), pPlayer->GetGUID());
            DeathSide_Training_Dummy_Hello(pPlayer, pCreature);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+9:
        case GOSSIP_ACTION_INFO_DEF+10:
        case GOSSIP_ACTION_INFO_DEF+11:
        {
            if (((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle)
            {
                switch (pCreature->GetEntry())
                {
                    case 555055:
                        pCreature->setFaction(42);
                        pPlayer->CastSpell(pPlayer, 54860, true); // Faction decreaser
                        break;
                    case 555056:
                        pCreature->setFaction(86);
                        pPlayer->CastSpell(pPlayer, 54861, true); // Faction decreaser
                        break;
                    case 555057:
                        pCreature->setFaction(113);
                        pPlayer->CastSpell(pPlayer, 54862, true); // Faction decreaser
                        break;
                    default:
                        pCreature->setFaction(7);
                }
            }
            else
                pCreature->setFaction(7); // Original faction will be 35 - green to everything
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->SetHealth(pPlayer->GetMaxHealth()); //restore hp
            pPlayer->SetPower(POWER_MANA, pPlayer->GetMaxPower(POWER_MANA)); // restore mana
            pPlayer->SetPower(POWER_RAGE, 0); //delete rage
            pPlayer->SetPower(POWER_ENERGY, pPlayer->GetMaxPower(POWER_ENERGY)); // restore energy
            pPlayer->RemoveArenaSpellCooldowns();
            switch (uiAction)
            {
                case GOSSIP_ACTION_INFO_DEF+9: // DD
                {
                    ((DeathSide_Training_DummyAI*)pCreature->AI())->BattleType = BTYPE_DD;
                    break;
                }
                case GOSSIP_ACTION_INFO_DEF+10: // Heal
                {
                    ((DeathSide_Training_DummyAI*)pCreature->AI())->BattleType = BTYPE_HEAL;
                    pPlayer->SetHealth(1);
                    break;
                }
                case GOSSIP_ACTION_INFO_DEF+11: // Tank
                {
                    ((DeathSide_Training_DummyAI*)pCreature->AI())->BattleType = BTYPE_TANK;
                    break;
                }
            }
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            pCreature->AddThreat(pPlayer, 1.0f);
            pCreature->AI()->AttackStart(pPlayer);
            if (((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle)
                ((DeathSide_Training_DummyAI*)pCreature->AI())->PlayerName = pPlayer->GetName();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 16:
        {
            if (((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle)
            {
                ((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle = false;
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_PERSONAL_FIGHT_NOW_OFF), pPlayer->GetGUID());
            }
            else
            {
                ((DeathSide_Training_DummyAI*)pCreature->AI())->IsPersonalBattle = true;
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_PERSONAL_FIGHT_NOW_ON), pPlayer->GetGUID());
            }
            DeathSide_Training_Dummy_Hello(pPlayer, pCreature);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 17:
        {
            pPlayer->PlayerTalkClass->ClearMenus();
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_ARMOR), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, "", 0, true);    // armor
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_FIRE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2, "", 0, true);    // resistance fire
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_NATURE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3, "", 0, true);    // resistance nature
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_ICE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4, "", 0, true);    // resistance frost
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_SHADOW), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5, "", 0, true);    // resistance shadow
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_ARCANE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6, "", 0, true);    // resistance arcane
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_CHANCE_DODGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12, "", 0, true);    // dodge chance
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_CHANCE_PARRY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13, "", 0, true);    // parry change
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_CHANCE_BLOCK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14, "", 0, true);    // block chance
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_BLOCK_DAMAGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15, "", 0, true);    // block value
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_LEVEL), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7, "", 0, true);    // level
            pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+18);
            pPlayer->SEND_GOSSIP_MENU(17,pCreature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 18:
        {
            DeathSide_Training_Dummy_Hello(pPlayer, pCreature);
            break;
        }
    }
    return true;
}

bool DeathSide_Training_Dummy_Code( Player *pPlayer, Creature *pCreature, uint32 uiSender, uint32 uiAction, const char* sCode )
{
    DeathSide_Training_Dummy_Code_Menu(pPlayer, pCreature, uiSender, uiAction, sCode);
    DeathSide_Training_Dummy_Hello(pPlayer, pCreature);
    return true;
}

void DeathSide_Training_Dummy_Code_Menu( Player *pPlayer, Creature *pCreature, uint32 uiSender, uint32 uiAction, const char* sCode )
{
    if (!sCode)
        return;
    int32 CodeNumber = (int32) atoi(sCode);

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1: // armor
        {
            char Whisper[128];
            if (CodeNumber < 0)
            {
                CodeNumber = -1;
                pCreature->SetResistance(SPELL_SCHOOL_NORMAL, 0);
                pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NORMAL, true);
                sprintf(Whisper, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_HAS_SET_PHYS_IMMUNITY));
            }
            else 
            {
                if (CodeNumber > 75000)
                    CodeNumber = 75000;
                pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NORMAL, false);
                pCreature->SetResistance(SPELL_SCHOOL_NORMAL, CodeNumber);
                sprintf(Whisper, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_HAS_SET_ARMOR), CodeNumber);
            }
            pCreature->Whisper(Whisper, pPlayer->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+2: // resistance
        case GOSSIP_ACTION_INFO_DEF+3: // resistance
        case GOSSIP_ACTION_INFO_DEF+4: // resistance
        case GOSSIP_ACTION_INFO_DEF+5: // resistance
        case GOSSIP_ACTION_INFO_DEF+6: // resistance
        {
            std::string Whisper = "";
            if (CodeNumber < 0)
            {
                Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_1); // Immunity has been set to
                CodeNumber = -1;
            }
            else
            {
                if (CodeNumber > 999)
                    CodeNumber = 999;
                Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_2);
                char buf[64];
                sprintf(buf, "%u", CodeNumber);
                Whisper += buf;
                Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_3);
            }

            switch (uiAction)
            {
                case GOSSIP_ACTION_INFO_DEF+2: // resistance
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_4); // fire magic
                    if (CodeNumber == -1)
                    {
                        pCreature->SetResistance(SPELL_SCHOOL_FIRE, 0);
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
                    }
                    else
                    {
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, false);
                        pCreature->SetResistance(SPELL_SCHOOL_FIRE, CodeNumber);
                    }
                    break;
                case GOSSIP_ACTION_INFO_DEF+3: // resistance
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_5);
                    if (CodeNumber == -1)
                    {
                        pCreature->SetResistance(SPELL_SCHOOL_NATURE, 0);
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
                    }
                    else
                    {
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, false);
                        pCreature->SetResistance(SPELL_SCHOOL_NATURE, CodeNumber);
                    }
                    break;
                case GOSSIP_ACTION_INFO_DEF+4: // resistance
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_6);
                    if (CodeNumber == -1)
                    {
                        pCreature->SetResistance(SPELL_SCHOOL_FROST, 0);
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
                    }
                    else
                    {
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, false);
                        pCreature->SetResistance(SPELL_SCHOOL_FROST, CodeNumber);
                    }
                    break;
                case GOSSIP_ACTION_INFO_DEF+5: // resistance
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_7);
                    if (CodeNumber == -1)
                    {
                        pCreature->SetResistance(SPELL_SCHOOL_SHADOW, 0);
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true);
                    }
                    else
                    {
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, false);
                        pCreature->SetResistance(SPELL_SCHOOL_SHADOW, CodeNumber);
                    }
                    break;
                case GOSSIP_ACTION_INFO_DEF+6: // resistance
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_8);
                    if (CodeNumber == -1)
                    {
                        pCreature->SetResistance(SPELL_SCHOOL_ARCANE, 0);
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
                    }
                    else
                    {
                        pCreature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, false);
                        pCreature->SetResistance(SPELL_SCHOOL_ARCANE, CodeNumber);
                    }
                    break;
            }
            pCreature->Whisper(Whisper.c_str(), pPlayer->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+7: // level
        {
            char Whisper[64];
            if (CodeNumber < 1)
                CodeNumber = 1;
            else if (CodeNumber > 255)
                CodeNumber = 255;
            pCreature->SetLevel(CodeNumber);
            sprintf(Whisper, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_HAS_SET_LEVEL), CodeNumber);
            pCreature->Whisper(Whisper, pPlayer->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+12: // dodge percent
        case GOSSIP_ACTION_INFO_DEF+13: // parry percent
        case GOSSIP_ACTION_INFO_DEF+14: // block percent
        {
            std::string Whisper = "";
            if (CodeNumber < 0)
                CodeNumber = 0;
            else if (CodeNumber > 200)
                CodeNumber = 200;
            Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_2); // empty
            char buf[64];
            sprintf(buf, "%u", CodeNumber);
            Whisper += buf;
            Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_10); //%% chance has been set
            CodeNumber -= 5;
            switch (uiAction)
            {
                case GOSSIP_ACTION_INFO_DEF+12: // dodge percent
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_11); // to dodge an attack.
                    pCreature->RemoveAurasDueToSpell(54742);
                    pCreature->CastCustomSpell(pCreature, 54742, &CodeNumber, NULL, NULL, true);
                    break;
                case GOSSIP_ACTION_INFO_DEF+13: // parry percent
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_12); // to parry an attack.
                    pCreature->RemoveAurasDueToSpell(20064);
                    pCreature->CastCustomSpell(pCreature, 20064, &CodeNumber, NULL, NULL, true);
                    break;
                case GOSSIP_ACTION_INFO_DEF+14: // block percent
                    Whisper += pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TRAINING_DUMMY_TEXT_13); // to block an attack.
                    pCreature->RemoveAurasDueToSpell(10021);
                    pCreature->CastCustomSpell(pCreature, 10021, &CodeNumber, NULL, NULL, true);
                    break;
            }
            pCreature->Whisper(Whisper.c_str(), pPlayer->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+15: // block value
        {
            char Whisper[128];
            if (CodeNumber < 0)
                CodeNumber = 0;
            else if (CodeNumber > 100000)
                CodeNumber = 100000;
            CodeNumber -= 35;
            pCreature->SetStat(STAT_STRENGTH, CodeNumber * 20);
            sprintf(Whisper, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_HAS_SET_BLOCK_DAMAGE), CodeNumber);
            pCreature->Whisper(Whisper, pPlayer->GetGUID());
            break;
        }
    }
    pPlayer->PlayerTalkClass->ClearMenus();
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_ARMOR), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, "", 0, true);    // armor
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_FIRE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2, "", 0, true);    // resistance fire
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_NATURE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3, "", 0, true);    // resistance nature
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_ICE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4, "", 0, true);    // resistance frost
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_SHADOW), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5, "", 0, true);    // resistance shadow
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_RESIST_ARCANE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6, "", 0, true);    // resistance arcane
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_CHANCE_DODGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12, "", 0, true);    // dodge chance
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_CHANCE_PARRY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13, "", 0, true);    // parry change
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_CHANCE_BLOCK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14, "", 0, true);    // block chance
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_BLOCK_DAMAGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15, "", 0, true);    // block value
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_LEVEL), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7, "", 0, true);    // level
    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+18);
    pPlayer->SEND_GOSSIP_MENU(17,pCreature->GetGUID());
}

CreatureAI* GetAI_DeathSide_Training_DummyAI(Creature* pCreature)
{
return new DeathSide_Training_DummyAI (pCreature);
}

 void AddSC_DeathSide_Training_Dummy()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Training_Dummy";
     newscript->GetAI = &GetAI_DeathSide_Training_DummyAI;
     newscript->pGossipHello          = &DeathSide_Training_Dummy_Hello;
     newscript->pGossipSelect          = &DeathSide_Training_Dummy_Gossip;
     newscript->pGossipSelectWithCode = &DeathSide_Training_Dummy_Code;
     newscript->RegisterSelf();
 }
