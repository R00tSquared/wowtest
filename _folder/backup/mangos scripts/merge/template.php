<?php

$tables_to_compare = [
	'playercreateinfo_spell',
	'playercreateinfo_spell',
	'playercreateinfo_item',
	'battleground_template',
	'game_tele',
	'item_template',
	'item_loot_template',
	'conditions',
	'spell_template',
	'disenchant_loot_template',
	'npc_trainer',
	'npc_vendor',
	'creature',
	'game_event_creature',
	'creature_template',
	'creature_questrelation',
	'creature_involvedrelation',
	'creature_loot_template',
	'gameobject',
	'game_event_gameobject',
	'gameobject_template',
	'gameobject_involvedrelation',
	'gameobject_questrelation',
	'quest_template',
	'npc_text',
	'game_event',
	'game_event_creature',
	'game_event_gameobject',
	'game_event_quest',
	'reference_loot_template',
	'reference_loot_template_names',
	'broadcast_text_locale',
	'gossip_texts',
	'mangos_string',
	'script_texts',
	'custom_texts',
	'locales_areatrigger_teleport',
	'locales_gameobject',
	'locales_gossip_menu_option',
	'locales_npc_text',
	'locales_page_text',
	'locales_points_of_interest',
	'locales_questgiver_greeting',
	'locales_trainer_greeting',	
	'locales_quest',
	'locales_item',
	'locales_creature',
];

$recreate_tables = [
	'spell_template_merged',
	'spell_analog_vice_versa',
	'spell_analog',
	'race_change_swap_spells',
	'race_change_swap_innate_rep',
	'race_change_swap_faction_rep',
	'race_change_swap_faction_quests',
	'race_change_swap_class_spells',
	'race_change_spells',
	'race_change_skins',
	'fakebot_names',
	'fakebot_locations_bkp',
	'fakebot_locations',
	'fakebot_levelcount_bkp',
	'fakebot_levelcount',
	'chest_announce',
	'autobroadcast',
	'command',
	'merge_lock',
	'fast_start_class_mask',
	'fast_start_create_spell',
	'fast_start_item_types',
	'fast_start_items',
	'fast_start_items_plus',
];

$replace_data_in_tables = [
	'playercreateinfo_spell',
	'playercreateinfo_spell',
	'playercreateinfo_item',
	'battleground_template',
	'game_tele',
];

// // locales need special impl
$locales_recreate = [
	'broadcast_text_locale',
	'locales_creature',
	'locales_item',
	'locales_gameobject',
	'locales_areatrigger_teleport',
	'locales_gossip_menu_option',
	'locales_npc_text',
	'locales_page_text',
	'locales_points_of_interest',
	'locales_quest',
	'locales_questgiver_greeting',
	'locales_trainer_greeting',
];

//other
// gossip_texts
// mangos_string
// script_texts

//check if non empty
// custom_texts

// deleted 1136
if ($realm_id == 3)
{
	$custom_items_entries = [2363,23796,4618,33573,33572,33570,30471,2554,3513,21233,23656,23700,23701,1199,5564,734,1698,1699,20366,32372,38234,7388,7986,7987,7988,16086,16102,16103,16104,35519,35520,35521,30796,35517,23698,8962,8963,8954,8955,8958,8960,8961,8964,8965,8966,23086,5172,2767,23689,8967,8968,8969,20020,28877,4966,21065,23961,695000,693427,693425,693424,693423,693422,693421,693420,693419,693418,693417,693416,693415,693413,693412,693408,693407,693404,693403,693402,693400,693392,693391,693390,693389,693388,693387,693386,693385,693384,693383,693382,693381,693380,693285,693284,693283,693282,693281,693280,693279,693278,693277,693276,693275,693274,693273,693272,693271,693270,693269,693268,693267,693266,693265,693264,693263,693262,693261,693260,693259,693258,693257,693256,693255,693254,693253,693252,693251,693250,693249,693248,693247,693246,693245,693244,693243,693241,693240,693239,693238,693237,693236,693234,693233,693232,693231,693229,693228,693227,693226,693225,693222,693218,693217,693215,693214,693212,693210,693209,693208,693207,693206,693205,693204,693201,693182,693181,693180,693179,693177,693176,693175,693174,693172,693167,693166,693165,693164,693163,693162,693161,693149,693148,693147,693146,693145,693144,693143,693142,693141,693140,693139,693138,693137,693136,693135,693134,693133,693132,693131,693130,693129,693128,693127,693126,693125,693124,693123,693122,693121,693120,693119,693118,693117,693116,693115,693114,693113,693112,693105,693103,693037,693036,693035,875,1042,1084,1691,3580,5049,5384,5697,5698,5857,6208,6209,10478,10594,12440,19932,21141,21811,22118,22124,22125,22126,22127,22129,23712,23861,23872,27318,27319,34519,7246,22475,23794,23224,10284,4420,23325,28482,26567,26562,26566,26563,26565,26561,26564,26568];
	// item_template
	// locales_item
	// item_loot_template	
	
	$conditions_types = [
	'100'
	];
	// conditions
	
	$npc_trainer_start_entry = 693000;
	// npc_trainer
	
	$disenchant_loot_template_start_entry = 100;
	// disenchant_loot_template	
	
	$reference_loot_template_start_entry = 693000;
	// reference_loot_template 
	// reference_loot_template_names

	$gameobject_template_start_entry = 690000;
	// gameobject_template
	// locales_gameobject
	// UNUSED: gameobject_involvedrelation
	// UNUSED: gameobject_template_addon
	// UNUSED: gameobject_questrelation	

	$quest_start_entry = 690000;
	// quest_template
	// locales_quest	
	
	$npc_vendor_start_entry = 693000;
	// npc_vendor
	
	$move_creature_relations = 1;
	$move_creature_loot = 1;
}
else
{
	$custom_items_entries = [734,1085,1086,1087,1088,1089,1090,1091,1092,1093,1095,1096,1099,1100,1101,1102,1199,1281,1698,1699,1924,2554,3513,4966,5049,5172,5564,7388,7986,7987,7988,8954,8955,8958,8960,8961,8962,8963,8964,8965,8966,8967,8968,8969,22475,23224,23656,23689,23698,23700,23701,23794,23796,28095,28096,28097,28098,30471,30796,33570,33572,33573,35517,35519,35520,35521,8166,695000,21233,20366,693035,693036,693037,693103,693161,693162,693163,693164,693165,693166,693167,695002,695005];
	// item_template
	// locales_item 
	// item_loot_template	
	
}

$gameobject_start_guid = 15500000;
// gameobject
// game_event_gameobject
// UNUSED: gameobject_spawn_entry
// UNUSED: gameobject_addon

$spell_template_start_id = 54844;
// spell_template

$creature_start_guid = 15000000;
// creature
// game_event_creature

$creature_template_start_entry = 690000;
// creature_template
// locales_creature
// creature_questrelation
// creature_involvedrelation
// creature_loot_template

$npc_text_start_id = 990000;
// locales_npc_text
// npc_text

$game_event_start_entry = 1500;
// game_event
// game_event_creature
// game_event_gameobject
// game_event_quest
// UNUSED: game_event_mail
// game_event_time
// UNUSED: game_event_creature_data

$mangos_string_start_entry = 10000;
// mangos_string