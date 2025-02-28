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
SDName: Item_Scripts
SD%Complete: 100
SDComment: Items for a range of different items. See content below (in script)
SDCategory: Items
EndScriptData */

/* ContentData
item_attuned_crystal_cores(i34368)  Prevent abuse(quest 11524 & 11525)
item_blackwhelp_net(i31129)         Quest Whelps of the Wyrmcult (q10747). Prevents abuse
item_draenei_fishing_net(i23654)    Hacklike implements chance to spawn item or creature
item_disciplinary_rod               Prevents abuse
item_nether_wraith_beacon(i31742)   Summons creatures for quest Becoming a Spellfire Tailor (q10832)
item_flying_machine(i34060,i34061)  Engineering crafted flying machines
item_gor_dreks_ointment(i30175)     Protecting Our Own(q10488)
item_muiseks_vessel                 Cast on creature, they must be dead(q 3123,3124,3125,3126,3127)
item_only_for_flight                Items which should only useable while flying
item_protovoltaic_magneto_collector Prevents abuse
item_razorthorn_flayer_gland        Quest Discovering Your Roots (q11520) and Rediscovering Your Roots (q11521). Prevents abuse
item_tame_beast_rods(many)          Prevent cast on any other creature than the intended (for all tame beast quests)
item_soul_cannon(i32825)            Prevents abuse of this item
item_sparrowhawk_net(i32321)        Quest To Catch A Sparrowhawk (q10987). Prevents abuse
item_voodoo_charm                   Provide proper error message and target(q2561)
item_vorenthals_presence(i30259)    Prevents abuse of this item
item_yehkinyas_bramble(i10699)      Allow cast spell on vale screecher only and remove corpse if cast sucessful (q3520)
item_zezzak_shard(i31463)           Quest The eyes of Grillok (q10813). Prevents abuse
item_inoculating_crystal            Quest Inoculating. Prevent abuse
EndContentData */

#include "precompiled.h"
#include "SpellMgr.h"
#include "Spell.h"
#include "WorldPacket.h"

/*#####
# item_only_for_flight
#####*/

bool ItemUse_item_only_for_flight(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    uint32 itemId = _Item->GetEntry();
    bool disabled = false;

    //for special scripts
    switch(itemId)
    {
       case 24538:
           if(player->GetCachedArea() != 3628)
               disabled = true;
               break;
       case 28132:
           if(player->GetCachedArea() != 3803)
               disabled = true;
               break;
       case 34489:
           if(player->GetCachedZone() != 4080)
               disabled = true;
               break;
    }

    // allow use in flight only
    if( player->IsTaxiFlying() && !disabled)
        return false;

    // error
    player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW,_Item,NULL);
    return true;
}

/*#####
# item_draenei_fishing_net
#####*/

//This is just a hack and should be removed from here.
//Creature/Item are in fact created before spell are sucessfully cast, without any checks at all to ensure proper/expected behavior.
bool ItemUse_item_draenei_fishing_net(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if( player->GetQuestStatus(9452) == QUEST_STATUS_INCOMPLETE )
    {
        GameObject* pGo = GetClosestGameObjectWithEntry(player, 181616, 10.0f);

        if(!pGo)
            return true;

        if( roll_chance_i(35) )
        {
            Creature *Murloc = player->SummonCreature(17102,player->GetPositionX() ,player->GetPositionY()+20, player->GetPositionZ(), 0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,10000);
            if( Murloc )
                Murloc->AI()->AttackStart(player);
        }
        else
        {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 23614, 1);
            if( msg == EQUIP_ERR_OK )
            {
                Item* item = player->StoreNewItem(dest,23614,true);
                if( item )
                    player->SendNewItem(item,1,false,true);
            }
            else
                player->SendEquipError(msg,NULL,NULL);
        }
    }
    return false;
}


/*#####
# item_nether_wraith_beacon
#####*/

bool ItemUse_item_nether_wraith_beacon(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if (player->GetQuestStatus(10832) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* Nether = player->SummonCreature(22408, player->GetPositionX(), player->GetPositionY() + 20, player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 180000))
            ((CreatureAI*)Nether->AI())->AttackStart(player);

        if (Creature* Nether = player->SummonCreature(22408, player->GetPositionX(), player->GetPositionY() - 20, player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 180000))
            ((CreatureAI*)Nether->AI())->AttackStart(player);
    }
    return false;
}

/*#####
# item_flying_machine
#####*/

bool ItemUse_item_flying_machine(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    uint32 itemId = _Item->GetEntry();
    if( itemId == 34060 )
        if( player->GetBaseSkillValue(SKILL_RIDING) >= 225 )
            return false;

    if( itemId == 34061 )
        if( player->GetBaseSkillValue(SKILL_RIDING) == 300 )
            return false;

    debug_log("TSCR: Player attempt to use item %u, but did not meet riding requirement",itemId);
    player->SendEquipError(EQUIP_ERR_ERR_CANT_EQUIP_SKILL,_Item,NULL);
    return true;
}

/*#####
# item_gor_dreks_ointment
#####*/

bool ItemUse_item_gor_dreks_ointment(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if( targets.getUnitTarget() && targets.getUnitTarget()->GetTypeId()==TYPEID_UNIT &&
        targets.getUnitTarget()->GetEntry() == 20748 && !targets.getUnitTarget()->HasAura(32578,0) )
        return false;

    player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW,_Item,NULL);
    return true;
}

/*#####
# item_tame_beast_rods
#####*/

bool ItemUse_item_tame_beast_rods(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    uint32 itemSpell = _Item->GetProto()->Spells[0].SpellId;
    uint32 cEntry = 0;

    if(itemSpell)
    {
        switch(itemSpell)
        {
            case 19548: cEntry =  1196; break;              //Ice Claw Bear
            case 19674: cEntry =  1126; break;              //Large Crag Boar
            case 19687: cEntry =  1201; break;              //Snow Leopard
            case 19688: cEntry =  2956; break;              //Adult Plainstrider
            case 19689: cEntry =  2959; break;              //Prairie Stalker
            case 19692: cEntry =  2970; break;              //Swoop
            case 19693: cEntry =  1998; break;              //Webwood Lurker
            case 19694: cEntry =  3099; break;              //Dire Mottled Boar
            case 19696: cEntry =  3107; break;              //Surf Crawler
            case 19697: cEntry =  3126; break;              //Armored Scorpid
            case 19699: cEntry =  2043; break;              //Nightsaber Stalker
            case 19700: cEntry =  1996; break;              //Strigid Screecher
            case 30646: cEntry = 17217; break;              //Barbed Crawler
            case 30653: cEntry = 17374; break;              //Greater Timberstrider
            case 30654: cEntry = 17203; break;              //Nightstalker
            case 30099: cEntry = 15650; break;              //Crazed Dragonhawk
            case 30102: cEntry = 15652; break;              //Elder Springpaw
            case 30105: cEntry = 16353; break;              //Mistbat
        }
        if( targets.getUnitTarget() && targets.getUnitTarget()->GetTypeId()==TYPEID_UNIT &&
            targets.getUnitTarget()->GetEntry() == cEntry )
            return false;
    }

    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_BAD_TARGETS);                // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target

    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

/*#####
# item_specific_target
#####*/

enum aliveMask
{
    T_ALIVE = 0x1,
    T_DEAD  = 0x2
};

#define MAX_TARGETS 4

bool ItemUse_item_specific_target(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    Unit *uTarget = targets.getUnitTarget() ? targets.getUnitTarget() : Unit::GetUnit(*player, player->GetSelection());

    uint32 iEntry = _Item->GetEntry();
    uint32 cEntry[MAX_TARGETS] = { 0, 0, 0, 0 };

    uint8 targetState = T_ALIVE & T_DEAD;

    switch(iEntry)
    {
        case 8149:  cEntry[0] = 7318; targetState = T_DEAD; break; // Voodoo Charm
        case 22783: cEntry[0] = 16329; break; // Sunwell Blade
        case 22784: cEntry[0] = 16329; break; // Sunwell Orb
        case 22962: cEntry[0] = 16518; break; // Inoculating Crystal
        case 30259: cEntry[0] = 20132; break; // Voren'thal's Presence
        case 30656: cEntry[0] = 21729; break; // Protovoltaic Magneto Collector
        case 31463: cEntry[0] = 19440; break; // Zezzak's Shard
        case 32321: cEntry[0] = 22979; break; // Sparrowhawk Net
        case 32825: cEntry[0] = 22357; break; // Soul Cannon
        case 34255: cEntry[0] = 24922; break; // Razorthorn Flayer Gland
        case 30251: cEntry[0] = 20058; break; // Rina's Diminution Powder
        case 23417: cEntry[0] = 16975; break; // Sanctified Crystal
        case 32698: cEntry[0] = 22181; break; // Wrangling Rope
        case 29513: cEntry[0] = 19354; break; // Staff of the Dreghood Elders
        case 22473: cEntry[0] = 15941; cEntry[1] = 15945; break;                        // Disciplinary Rod
        case 34368: cEntry[0] = 24972; targetState = T_DEAD; break;                     // Attuned Crystal Cores
        case 32680: cEntry[0] = 23311; targetState = T_ALIVE; break;                    // Booterang
        case 34257: cEntry[0] = 24918; targetState = T_ALIVE; break;                    // Fel Siphon
        case 28547: cEntry[0] = 18881; cEntry[1] = 18865; break;                        // Elemental power extractor
        case 12284: cEntry[0] = 7047;  cEntry[1] = 7048;  cEntry[2] = 7049; break;      // Draco-Incarcinatrix 900
        case 23337: cEntry[0] = 16880; targetState = T_ALIVE; break;                    // Cenarion Antidote
        case 29818: cEntry[0] = 20774; targetState = T_ALIVE; break;                    // Energy Field Modulator
        case 13289: cEntry[0] = 10384; cEntry[1] = 10385; cEntry[2] = 11122; targetState = T_ALIVE; break; // Egan's Blaster
        case 25552: cEntry[0] = 17148; cEntry[1] = 17147; cEntry[2] = 17146; targetState = T_DEAD; break; // Warmaul Ogre Banner
        case 33108: cEntry[0] = 4393; cEntry[1] = 4394; targetState = T_ALIVE; break;
        case 30651: cEntry[0] = 21254; targetState = T_ALIVE; break; //  Dertrok's First Wand
        case 30652: cEntry[0] = 21254; targetState = T_ALIVE; break; //  Dertrok's Second Wand
        case 30653: cEntry[0] = 21254; targetState = T_ALIVE; break; //  Dertrok's Third Wand
        case 30654: cEntry[0] = 21254; targetState = T_ALIVE; break; //  Dertrok's Fourth Wand
        case 29817: cEntry[0] = 20610; cEntry[1] = 20777; break;    // Talbuk Tagger
        case 23394: cEntry[0] = 16847; targetState = T_ALIVE; break; // Healing Salve
        case 31518: cEntry[0] = 21326; targetState = T_ALIVE; break; // Exorcism Feather
        case 31129: cEntry[0] = 21387; targetState = T_ALIVE; break; // Blackwhelp Net
        case 24278: cEntry[0] = 17664; targetState = T_ALIVE; break; // Flare Gun
        case 9619: cEntry[0] = 5300;  cEntry[1] = 5304; cEntry[2] = 5305; cEntry[3] = 5306; targetState = T_DEAD; break;
        case 9606: cEntry[0] = 7584; targetState = T_DEAD; break;
        case 9618: cEntry[0] = 2927;  cEntry[1] = 2928; cEntry[2] = 2929; cEntry[3] = 7808; targetState = T_DEAD; break;
        case 9620: cEntry[0] = 5276;  cEntry[1] = 5278; targetState = T_DEAD; break;
        case 9621: cEntry[0] = 5357;  cEntry[1] = 5358; cEntry[2] = 14640; cEntry[3] = 14604; targetState = T_DEAD; break;
        case 22432: cEntry[0] = 6498; cEntry[1] = 6499; cEntry[2] = 6500; cEntry[3] = 6584; targetState = T_ALIVE; break;
        case 31828: cEntry[0] = 22507; cEntry[1] = 22506; cEntry[3] = 22432; targetState = T_ALIVE; break;
    }

    if(uTarget && uTarget->GetTypeId() == TYPEID_UNIT)
    {
        bool properTarget = false;
        for(uint8 i = 0; i < MAX_TARGETS; i++)
        {
            if(uTarget->GetEntry() == cEntry[i])
            {
                properTarget = true;
                break;
            }
        }

        if(properTarget)
        {
            switch(targetState)
            {
                case(T_ALIVE & T_DEAD):
                    return false;
                case T_ALIVE:
                {
                    if(uTarget->isAlive())
                        return false;
                    else
                    {
                        WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
                        data << uint32(_Item->GetEntry());                      // itemId
                        data << uint8(SPELL_FAILED_TARGETS_DEAD);               // reason
                        player->GetSession()->SendPacket(&data);                // send message: Invalid target
                        player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);
                        return true;
                    }
                }
                case T_DEAD:
                {
                    if(uTarget->getDeathState() == CORPSE)
                        return false;
                    else if (uTarget->getDeathState() != DEAD)
                    {
                        WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
                        data << uint32(_Item->GetEntry());                      // itemId
                        data << uint8(SPELL_FAILED_TARGET_NOT_DEAD);            // reason
                        player->GetSession()->SendPacket(&data);                // send message: Invalid target
                        player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);
                        return true;
                    }
                    // if deathstate == dead corpse should be invisible and untargettable, so invalid target
                }
            }
        }
    }

    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_BAD_TARGETS);                // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target

    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

/*#####
# item_rood_rofl - custom event mount
#####*/

bool ItemUse_item_rood_rofl(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    // player must have riding skill
    if(!player->HasSkill(762))
    {
        player->SendEquipError(EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM,_Item,NULL);
        return true;
    }
    // player must have possibility to use 60% flying mount
    if(player->GetBaseSkillValue(762) < 225)
    {
        player->SendEquipError(EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM,_Item,NULL);
        return true;
    }

    if (player->HasAuraType(SPELL_AURA_MOUNTED))
        player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    return false;
}

/*#####
# item_chest_of_containment_coffers
#####*/

#define MOB_RIFT_SPAWN          6492
#define SPELL_SELF_STUN_30SEC   9032

bool ItemUse_item_chest_of_containment_coffers(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    std::list<Creature*> SpawnList;
    Hellground::AllCreaturesOfEntryInRange u_check(player, MOB_RIFT_SPAWN, 20.0);
    Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(SpawnList, u_check);
    Cell::VisitAllObjects(player, searcher, 20.0);

    if(!SpawnList.empty())
    {
        for(std::list<Creature*>::iterator i = SpawnList.begin(); i != SpawnList.end(); ++i)
        {
            if((*i)->HasAura(SPELL_SELF_STUN_30SEC))
                return false;
        }
    }

    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_BAD_TARGETS);                // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target

    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

/*#####
# item_murloc_tagger
#####*/

bool ItemUse_item_murloc_tagger(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if (player->GetQuestStatus(9629) == QUEST_STATUS_INCOMPLETE)
    {
        if(!targets.getUnitTarget()->HasAura(30877,0))
            return false;
    }
    player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW,_Item,NULL);
    return true;
}

/*#####
# item_mana_remnants
#####*/

bool ItemUse_item_mana_remnants(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if (player->GetQuestStatus(11523) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11496) == QUEST_STATUS_INCOMPLETE)
    {
        std::list<Creature*> SpawnList;
        Hellground::AllCreaturesOfEntryInRange u_check(player, 24980, 8.0);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(SpawnList, u_check);
        Cell::VisitAllObjects(player, searcher, 8.0);
    
        if(!SpawnList.empty())
            return false;
    }
    player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW,_Item,NULL);
    return true;
}

bool ItemUse_item_broggs_totem(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    Unit *target = targets.getUnitTarget();

    if ((player->GetQuestStatus(11161) == QUEST_STATUS_INCOMPLETE) && (target) && (target->GetTypeId() == TYPEID_UNIT) && (target->isDead()) && 
        (!target->HasAura(55160,0)) && ((target->GetEntry() == 4331) || (target->GetEntry() == 4328) || (target->GetEntry() == 4329)))
    {
        target->AddAura(55160, target);
        return false;
    }
    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_BAD_TARGETS);                // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target

    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

bool ItemUse_item_10699(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    Unit *target = targets.getUnitTarget();

    if ((player->GetQuestStatus(3520) == QUEST_STATUS_INCOMPLETE) && (target) && (target->GetTypeId() == TYPEID_UNIT) && 
        (!target->HasAura(55160,0)) && ((target->GetEntry() == 5307) || (target->GetEntry() == 5308)))
    {
        target->AddAura(55160, target);
        return false;
    }
    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_BAD_TARGETS);                // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target

    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

bool ItemUse_item_28550(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if (player->GetQuestStatus(10233) == QUEST_STATUS_INCOMPLETE)
    {
        std::list<Creature*> balista;
        Hellground::AllCreaturesOfEntryInRange u_check(player, 19723, 10);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(balista, u_check);
        Cell::VisitAllObjects(player, searcher, 10);

        std::list<Creature*> tent;
        Hellground::AllCreaturesOfEntryInRange u_check1(player, 19724, 10);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher1(tent, u_check1);
        Cell::VisitAllObjects(player, searcher1, 10);

        balista.merge(tent);
    
        if(!balista.empty())
        {
            for (std::list<Creature*>::iterator itr = balista.begin(); itr != balista.end(); ++itr)
            {
                if ((*itr)->isAlive())
                    return false;
            }
        }
    }
    WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
    data << uint32(_Item->GetEntry());                      // itemId
    data << uint8(SPELL_FAILED_BAD_TARGETS);                // reason
    player->GetSession()->SendPacket(&data);                // send message: Invalid target
    player->SendEquipError(EQUIP_ERR_NONE,_Item,NULL);      // break spell
    return true;
}

bool ItemUse_ooze_buster(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if (player->GetQuestStatus(11174) == QUEST_STATUS_INCOMPLETE)
    {
        if(player->HasAura(42490,0))
            return false;
    }
    player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW,_Item,NULL);
    return true;
}

bool ItemUse_GH_teleport(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if (!player->IsGuildHouseOwnerMember())
    {
        ChatHandler(player).PSendSysMessage(15514);
        return false;
    }

    int32 bpaction = 8999;
    player->CastCustomSpell(player, 55116, &bpaction, NULL, NULL, false);
    return true;
}

void AddSC_item_scripts()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="item_ooze_buster";
    newscript->pItemUse = &ItemUse_ooze_buster;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_only_for_flight";
    newscript->pItemUse = &ItemUse_item_only_for_flight;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_draenei_fishing_net";
    newscript->pItemUse = &ItemUse_item_draenei_fishing_net;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_nether_wraith_beacon";
    newscript->pItemUse = &ItemUse_item_nether_wraith_beacon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_flying_machine";
    newscript->pItemUse = &ItemUse_item_flying_machine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_gor_dreks_ointment";
    newscript->pItemUse = &ItemUse_item_gor_dreks_ointment;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_tame_beast_rods";
    newscript->pItemUse = &ItemUse_item_tame_beast_rods;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_specific_target";
    newscript->pItemUse = &ItemUse_item_specific_target;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_rood_rofl";
    newscript->pItemUse = &ItemUse_item_rood_rofl;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_chest_of_containment_coffers";
    newscript->pItemUse = &ItemUse_item_chest_of_containment_coffers;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_murloc_tagger";
    newscript->pItemUse = &ItemUse_item_murloc_tagger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_mana_remnants";
    newscript->pItemUse = &ItemUse_item_mana_remnants;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_broggs_totem";
    newscript->pItemUse = &ItemUse_item_broggs_totem;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_10699";
    newscript->pItemUse = &ItemUse_item_10699;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_28550";
    newscript->pItemUse = &ItemUse_item_28550;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_guild_house_teleport";
    newscript->pItemUse = &ItemUse_GH_teleport;
    newscript->RegisterSelf();
}

