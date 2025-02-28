-- This file should only be run BEFORE 2.1.0 is released

-- ----------------------------------
-- Black Temple released in Patch 2.1
-- ----------------------------------
UPDATE areatrigger_teleport SET required_level=80 WHERE id IN (4598);
UPDATE creature SET spawnMask=1 WHERE guid=180001; -- Spawn Raid Messenger (Black Temple)

-- -----------------------------------------------------------
-- Attunements for The Eye and SSC were removed in Patch 2.1.2
-- -----------------------------------------------------------
-- Serpentshrine Cavern
-- also requires http://www.wowhead.com/spell=39145/the-mark-of-vashj cast on player to enter (parts waterfall at entrance)
UPDATE areatrigger_teleport SET required_quest_done=10901 WHERE id IN(4416);
-- The Eye
UPDATE areatrigger_teleport SET required_item=31704 WHERE id=4470;

-- -----------------------------------------------------------
-- Dire Maul:
-- Patch 2.1.0 (22-May-2007): The three Guards no longer buff players above level 63.
-- https://wowwiki.fandom.com/wiki/Dire_Maul_tribute_run
-- -----------------------------------------------------------
UPDATE gossip_menu_option SET condition_id=0 WHERE menu_id IN(5735,5731,5733);

-- ---------------------------------------------------------
-- Death Ravager becomes untamable in Patch 2.1
-- Hunters who already tamed prior were allowed to keep it
-- ---------------------------------------------------------
UPDATE creature_template SET CreatureTypeFlags=1 WHERE entry=17556;

-- Schematic: Fused Wiring
-- Patch 2.1.0 (22-May-2007): An engineering recipe to make Fused Wiring can be found in both Everlook and Shattrath.
DELETE FROM npc_vendor WHERE item=32381;

-- -------------------------------------
-- Cro Threadstrong had different texts on different patches
-- -------------------------------------
-- 2.0 texts guessed based on wowwiki and wowhead (http://wowwiki.wikia.com/wiki/Cro_Threadstrong) (http://www.wowhead.com/npc=19196/cro-threadstrong#comments)

DELETE FROM `dbscripts_on_creature_movement` WHERE `id` BETWEEN 1919601 AND 1919603;
INSERT INTO `dbscripts_on_creature_movement` (`id`,`delay`,`command`,`datalong`,`datalong2`,`dataint`,`dataint2`,`dataint3`,`dataint4`,`buddy_entry`,`search_radius`,`data_flags`,`comments`) VALUES
(1919601,0,0,0,0,2000001180,0,0,0,0,0,0,'Cro Threadstrong - Random text'),
(1919602,5000,1,6,0,0,0,0,0,0,0,0,'Cro Threadstrong - OneShotQuestion'),
(1919603,5000,0,0,0,2000001185,2000001186,2000001187,2000001188,0,0,0,'Cro Threadstrong - Random text'),
(1919603,12000,32,1,0,0,0,0,0,19223,50,0,'Granny Smith - Pause Waypoints'),
(1919603,12000,28,0,0,0,0,0,0,19223,50,0,'Granny Smith - UNIT_STAND_STATE_STAND'),
(1919603,12000,1,0,0,0,0,0,0,19223,50,0,'Granny Smith - ONESHOT_NONE'),
(1919603,13000,36,0,0,0,0,0,0,19223,50,3,'Granny Smith - Face Cro Threadstrong'),
(1919603,14000,0,0,0,2000001189,2000001190,2000001191,2000001192,19223,50,3,'Granny Smith - Random text'),
(1919603,22000,1,6,0,0,0,0,0,19223,50,0,'Granny Smith - OneShotQuestion'),
(1919603,26000,32,0,0,0,0,0,0,19223,50,0,'Granny Smith - Resume Waypoints');

DELETE FROM `dbscript_string` WHERE `entry` BETWEEN 2000001180 AND 2000001188;
INSERT INTO `dbscript_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`, `sound`, `type`, `language`, `emote`, `broadcast_text_id`, `comment`) VALUES 
(2000001180, 'Who is this fruit vendor to make such a bold move? He''s brought in an ogre for support.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 22057, 'Cro Threadstrong (Entry: 19196)'),
(2000001181, 'Unused in 2.0', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 'Cro Threadstrong (Entry: 19196)'),
(2000001182, 'Unused in 2.0', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 'Cro Threadstrong (Entry: 19196)'),
(2000001183, 'Unused in 2.0', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 'Cro Threadstrong (Entry: 19196)'),
(2000001184, 'Unused in 2.0', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 'Cro Threadstrong (Entry: 19196)'),

(2000001185, 'Does this fruit vendor not value his life? YOU ARE RUNNING OUT OF TIME FRUIT VENDOR!!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 16297, 'Cro Threadstrong (Entry: 19196)'),
(2000001186, 'HA!  I CRUSHED AN APPLE, FRUIT VENDOR!  THIS WILL NOT BE THE LAST!!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 16298, 'Cro Threadstrong (Entry: 19196)'),
(2000001187, 'FRUIT VENDOR!!! Your cart is still in our way!  We will give you one more hour to move it from our area.  Do not test our patience anymore!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 16296, 'Cro Threadstrong (Entry: 19196)'),
(2000001188, 'IF WAR IS WHAT YOU WANT, WAR IS WHAT YOU SHALL GET, FRUIT VENDOR!  WE WILL SEE WHO SELLS MORE OF YOUR APPLES!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 27711, 'Cro Threadstrong (Entry: 19196)');

-- ------------------------------------------------------------------------------------------
-- There were no refugees standing next to Spymistress Mehlisah Highcrown in patch 2.0 (Unknown when they were added) (https://youtu.be/LgXTlZ_Iilk?t=2m19s)
-- ------------------------------------------------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE guid IN(SELECT guid FROM pool_creature WHERE pool_entry IN(104,105));
-- She also didn't do emotes on her own
DELETE FROM creature_ai_scripts WHERE creature_id=18893;
UPDATE creature_template SET AIName='' WHERE entry=18893;

-- ----------------------------
-- DRUID EPIC FLIGHT FORM QUEST
-- ----------------------------
UPDATE quest_template SET minlevel = 80 WHERE entry IN (10955,11011);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry=32449; -- Essence-Infused Moonstone

-- ------------
-- (DAILY) QUESTS
-- ------------

-- Ethereum Prison
UPDATE quest_template SET minlevel = 80 WHERE entry IN (10969,10970,10971,10972,10973,10974,10975,10976,10977,10982,10981);

-- Hellfire Fortifications 10106,10110
-- https://wow.gamepedia.com/Hellfire_Fortifications_(Alliance_daily)
-- Originally granted 250 reputation, was hotfixed to 150 on some unknown date.
UPDATE `quest_template` SET `RewRepValue1` = 250 WHERE `entry` IN (10106,10110);

-- https://tbc.wowhead.com/quest=8367/for-great-honor#comments:id=5314212
-- Transition to Black Temple Content increased Honor gain
UPDATE `quest_template` SET `RewHonorableKills` = 15 WHERE `entry` IN (8367,8388,8385,8371);

-- -----------------------------------
-- Disable The Black Temple Attunments 
-- -----------------------------------
UPDATE quest_template SET minlevel = 80 WHERE entry IN (10944,10959);

-- -----------------------------------
-- ITEM SPELL & DISENCHANTMENT CHANGES
-- -----------------------------------
UPDATE item_template SET SellPrice = 0 WHERE entry IN (30025, 28659,19108, 19107);
UPDATE item_template SET RequiredDisenchantSkill = -1, DisenchantID=0 WHERE entry IN (27550,19108,31272,31303,30684,30857,31148);

-- --------------------
-- PROFESSIONS
-- -------------------
-- all consumable elixirs were only allowed to stack to 5 instead of 20 prior to this patch?
-- Noggenfogger at least should be an exception to this, and possibly others too
UPDATE item_template SET stackable = 5 WHERE class = 0 AND subclass = 2 AND Stackable = 20 AND entry NOT IN (8529);

UPDATE creature_template SET SkinningLootID = 0 WHERE entry IN (14476,5347);

-- Demons (CreatureType = 3) dropped Mote of Shadow befor some loot rework prior or at 2.1
DELETE FROM `creature_loot_template` WHERE `entry` IN (19852,19853,20141,20394,21021) AND `item` = 22577;
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `comments`) VALUES
/*
25.0971	Cyber-Rage Forgelord	16943
25.1772	Mo'arg Doomsmith	16944
18.75	Mo'arg Engineer	16945
18.3999	Mo'arg Forgefiend	16946
18.2602	Terrorfiend	16951
22.9317	Anger Guard	16952
30.7692	Dread Tactician	16959
17.9828	Sister of Grief	16960
10.5905	Rogue Voidwalker	16974
10.7616	Uncontrolled Voidwalker	16975
18.4125	Collapsing Voidwalker	17014
18.2431	Gan'arg Tinkerer	17151
23.9619	Felguard Legionnaire	17152
18.6912	Voidspawn	17981
11.4772	Demos, Overseer of Hate	18535
11.0669	Xirkos, Overseer of Fear	18536
12.2023	Mo'arg Master Planner	18567
25.0903	Subjugator Vaz'shir	18660
23.9801	Terrorguard	18661
25.9	Vorakem Doomspeaker	18679
25	Voidhunter Yar	18683
25	Ambassador Jerrikar	18695
1.0615	Unstable Voidwraith	18869
1.0194	Voidshrieker	18870
16.5601	Felguard Destroyer	18977
18.4278	Arazzius the Cruel	19191
18.8993	Mistress of Doom	19192
14.8201	Warbringer Arix'Amal	19298
42.4242	Deathwhisperer	19299
45	Nexus Terror	19307
19.0751	Arzeth the Merciless	19354
18.6361	Vacillating Voidcaller	19527
60	Dimensius the All-Devouring	19554
22.4417	Doomclaw	19738
24.6001	Wrathwalker	19740
22.5905	Dreadwarden	19744
24.1471	Baelmon the Hound-Master	19747
24.4136	Deathforge Tinkerer	19754
24.7892	Mo'arg Weaponsmith	19755
24.0654	Deathforge Smith	19756
86.6148	Infernal Soul	19757
71.0526	Newly Crafted Infernal	19759
87.3813	Cooling Infernal	19760
24.2552	Illidari Dreadbringer	19799
24.1713	Illidari Painlasher	19800
5.0133	Illidari Agonizer	19801
24.0875	Illidari Shocktrooper	19802
14.1258	Levixus	19847
*/
(19852, 22577, 20, 0, 1, 2, 'Mote of Shadow'), 		-- Artifact Seeker			https://www.wowhead.com/npc=19852/artifact-seeker#comments:id=90133
(19853, 22577, 24.4787, 0, 1, 2, 'Mote of Shadow'), -- Felblade Doomguard		https://web.archive.org/web/20080220014745/http://wow.allakhazam.com:80/db/mob.html?wmob=19853
/*
24.6786	Doomforge Engineer	19960
25.2132	Doomforge Attendant	19961
24.2489	Doomcryer	19963
25	Dreadforge Servant	19971
24.2169	Deathforge Over-Smith	19978
23.7478	Deathforge Technician	19979
25.8083	Void Terror	19980
2.55	Crystalcore Mechanic	20052
18.0901	Culuthas	20138
*/
(20141, 22577, 35, 0, 1, 2, 'Mote of Shadow'), 		-- Hound of Culuthas		https://web.archive.org/web/20110814110421/http://www.wowhead.com/npc=20141#comments:id=35080
/*
24.5929	Pentatharon	20215
45	Nexus Terror (1)	20265
24.2116	Mo'arg Warp-Master	20326
*/
(20394, 22577, 35, 0, 1, 2, 'Mote of Shadow'), 		-- Eye of Culuthas
/*
24.7376	Warp-Gate Engineer	20404
21.6328	Veneratus the Many	20427
17.9019	Arzeth the Powerless	20680
25.3417	Prophetess Cavrylin	20683
17.1907	Razorsaw	20798
17.644	Forgemaster Morug	20800
16.1895	Silroth	20801
22.8533	Overmaster Grindgarr	20803
35	Negaton Warp-Master	20873
35	Negaton Screamer	20875
4.66	Deathforge Imp	20887
25.1286	Ironspine Forgelord	20928
24.8691	Wrath Lord	20929
24.7195	Hatecryer	20930
*/
(21021, 22577, 24.569, 0, 1, 2, 'Mote of Shadow'); 	-- Scorch Imp				https://www.wowhead.com/npc=21021/scorch-imp#comments:id=96234
/*
19.2097	Warbringer Razuun	21287
23.2102	Painmistress Gabrissa	21309
24.7891	Terrormaster	21314
24.8745	Illidari Shadowstalker	21337
23.4474	Overseer Ripsaw	21499
15.5938	Morgroron	21500
17.2355	Makazradon	21501
27.2059	Azaloth	21506
24.4969	Death's Might	21519
23.9554	Illidari Jailor	21520
35	Negaton Screamer (1)	21604
35	Negaton Warp-Master (1)	21605
24.7161	Illidari Satyr	21656
24.3134	Illidari Overseer	21808
24.7238	Zandras	21827
25.1188	Terrorguard Protector	21923
25.4237	Enslaved Doomguard	21963
33.7932	Fear Whisperer	22201
39.5604	Nightmare Imp	22202
20	Eredar Stormbringer	22283
34.4291	Darkflame Infernal	22289
42.3077	Throne-Guard Highlord	22297
37.8378	Throne-Guard Sentinel	22301
66.6667	Throne-Guard Champion	22302
70.9459	Throne Hound	22303
31.383	Nightmare Weaver	22325
33.6538	Terror-Fire Guardian	22327
50	Reth'hedron the Subduer	22357
31.384	Wrath Fiend	22392
*/

-- Patch ???
-- Add Arcane Crystal to Khorium Vein 181557 which was removed at some point
-- https://web.archive.org/web/20080522030133/http://wow.allakhazam.com:80/db/object.html?wobject=2993
DELETE FROM `gameobject_loot_template` WHERE `entry` = 18363 AND `item` = 12363;
INSERT INTO `gameobject_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
(18363, 12363, 2, 1, 1, 1, 0, 'Arcane Crystal');

-- --------------------------------------
-- Enchanting
-- --------------------------------------
-- Formula: Enchant Bracer - Major Defense Dropped From Ethereum Researcher 20456 Prior to 2.1.0
DELETE FROM creature_loot_template WHERE item=22530 AND entry IN (22822,20456);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
('20456','22530','1.5','0','1','1','203','Formula: Enchant Bracer - Major Defense');

-- --------------------------------------
-- Formula: Enchant Weapon - Major Striking - Not sold until 2.1
-- --------------------------------------
-- http://www.wowhead.com/item=22552/formula-enchant-weapon-major-striking#comments
-- Wowhead comments:
-- "As of 2.1 this formula is sold by Karaaz, Consortium Quartermaster."
DELETE FROM npc_vendor WHERE item=22552;
DELETE FROM npc_vendor_template WHERE item=22552;

-- --------------------------------------
-- Engineering
-- --------------------------------------
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (32479,32478,32494,32461,32476,32475,32480,32473,32474,32472,32495);
DELETE FROM npc_trainer_template WHERE spell IN (41311,41318,41317,41320,40274,41314,41315,41316,41319,41312,41321);

-- Frost Grenade
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry = 32413;
DELETE FROM npc_trainer_template WHERE spell = 39973;

-- Icy Blasting Primers
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry = 32423;
DELETE FROM npc_trainer_template WHERE spell = 39971;

-- Gyro-balanced Khorium Destroyer
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry = 32756;
DELETE FROM npc_trainer_template WHERE spell = 41307;

-- Schematic: Elemental Seaforium Charge
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (23819);
DELETE FROM npc_vendor_template WHERE item = 23874;

-- --------------------------------------
-- Jewelcrafting
-- --------------------------------------
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (32836,32833,32772,32776,32508,32774);
DELETE FROM npc_trainer_template WHERE spell IN (41429,41420,41414,41418,40514,41415);

-- Several gems and their recipes disabled. Added in 2.1
-- Patch 2.1.0 (2007-05-22): Added.
DELETE FROM creature_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM disenchant_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM fishing_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM gameobject_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM item_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM mail_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM pickpocketing_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM prospecting_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM reference_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM skinning_loot_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM npc_vendor_template WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
DELETE FROM npc_vendor WHERE item IN(31878,31876,31877,31879,32411,32412,33622,31875);
-- Design: Veiled Noble Topaz - 31878
-- Design: Balanced Nightseye - 31876
-- Design: Infused Nightseye - 31877
-- Design: Wicked Noble Topaz - 31879
-- Design: Thundering Skyfire Diamond - 32411
-- Design: Relentless Earthstorm Diamond - 32412 and 33622
-- Design: Great Dawnstone - 31875
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(31878,31876,31877,31879,32411,32412,33622,31875);
-- Veiled Noble Topaz - 31867
-- Balanced Nightseye - 31863
-- Infused Nightseye - 31865
-- Wicked Noble Topaz - 31868
-- Thundering Skyfire Diamond - 32410
-- Relentless Earthstorm Diamond - 32409
-- Great Dawnstone - 31861
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(31867,31863,31865,31868,32410,32409,31861);

-- [Design: Mystic Dawnstone] - Added in 2.1.2
-- Recipe Item ID: 24208, Gem Item ID: 24053
-- http://wowwiki.wikia.com/Patch_2.1.2
-- "A recipe for a superior resilience gem has been added and is rumored to be held by the residents of Halaa."
DELETE FROM npc_vendor WHERE item=24208;
DELETE FROM npc_vendor_template WHERE item=24208;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(24208, 24053);

-- --------------------------------------
-- Alchemy
-- --------------------------------------

-- -----------------------------------------------------------------------------
-- Recipe: Earthen Elixir was not sold until 2.1
-- -----------------------------------------------------------------------------
DELETE FROM item_loot_template WHERE item IN(32063);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(32063);
DELETE FROM npc_vendor WHERE item=32070;
DELETE FROM npc_vendor_template WHERE item=32070;

-- -----------------------------------------------------------------------------
-- Elixir of Draenic Wisdom was not sold until 2.1
-- -----------------------------------------------------------------------------
DELETE FROM item_loot_template WHERE item IN(32067);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(32067);
DELETE FROM npc_trainer_template WHERE spell IN (39638);
DELETE FROM npc_trainer WHERE spell IN (39638);

-- -----------------------------------------------------------------------------
-- Recipe: Elixir of Ironskin
-- -----------------------------------------------------------------------------
DELETE FROM item_loot_template WHERE item IN(32068);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(32071,32068);
DELETE FROM npc_vendor WHERE item=32071;
DELETE FROM npc_vendor_template WHERE item=32071;

-- -----------------------------------------------------------------------------
-- Elixir of Major Fortitude
-- -----------------------------------------------------------------------------
DELETE FROM item_loot_template WHERE item IN(32062);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(32062);
DELETE FROM npc_trainer_template WHERE spell IN (39636);
DELETE FROM npc_trainer WHERE spell IN (39636);

-- -----------------------------------------------------------------------------
-- Cauldrons were added in 2.1
-- -----------------------------------------------------------------------------
-- [Cauldron of Major Arcane Protection]
-- Item ID: 32839, Create Spell ID: 41458

-- [Cauldron of Major Fire Protection]
-- Item ID: 32849, Create Spell ID: 41500

-- [Cauldron of Major Frost Protection]
-- Item ID: 32850, Create Spell ID: 41501

-- [Cauldron of Major Nature Protection]
-- Item ID: 32851, Create Spell ID: 41502

-- [Cauldron of Major Shadow Protection]
-- Item ID: 32852, Create Spell ID: 41503

DELETE FROM skill_discovery_template WHERE spellId IN(41458,41500,41501,41502,41503);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(32839,32849,32850,32851,32852);

-- Plans: Boots of the Protector - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30323);

-- Plans: Red Belt of Battle - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30322);

-- Plans: Red Havoc Boots - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30324);

-- ----------------------------------------------------------
-- LEATHERWORKING
-- ----------------------------------------------------------

-- Belt of Deep Shadow (30040) - Spell: 36351
-- +3 Nether Vortex, -10 Primal Air
-- UPDATE spell_template SET ReagentCount1=5, Reagent5=0, ReagentCount5=0 WHERE id=36351;
-- It also didn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30302);

-- Belt of Natural Power (30042) - Spell: 36349
-- +3 Nether Vortex, -6 Primal Life, -10 Primal Air
-- UPDATE spell_template SET ReagentCount1=5, ReagentCount3=4, Reagent5=0, ReagentCount5=0 WHERE id=36349;
-- It also didn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30301);

-- Belt of the Black Eagle (30046) - Spell: 36352
-- +2 Nether Vortex, -6 Primal Air, -4 Wind Scales
-- UPDATE spell_template SET ReagentCount1=4, ReagentCount3=4, ReagentCount4=6 WHERE id=36352;
-- It also didn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30303);

-- Monsoon Belt (30044) - Spell: 36353
-- +2 Nether Vortex, -6 Primal Water
-- UPDATE spell_template SET ReagentCount1=4, ReagentCount3=4 WHERE id=36353;
-- It also didn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30304);

-- Pattern: Hurricane Boots - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30308);

-- Pattern: Boots of Natural Grace - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30305);

-- Pattern: Boots of the Crimson Hawk - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30307);

-- Pattern: Boots of Utter Darkness - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30306);

-- ----------------------------------------------------------
-- TAILORING
-- ----------------------------------------------------------

-- Belt of Blasting (30038) - Spell: 36315
-- +3 Nether Vortex, -13 Primal Fire
-- UPDATE spell_template SET ReagentCount2=5, ReagentCount3=2 WHERE id=36315;
-- It also didn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30280);

-- Belt of the Long Road (30036) - Spell: 36316
-- +3 Nether Vortex, -8 Primal Life
-- UPDATE spell_template SET ReagentCount2=5, ReagentCount3=2 WHERE id=36316;
-- It also didn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30281);

-- Pattern: Boots of Blasting - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30282);

-- Pattern: Boots of the Long Road - Doesn't drop until 2.1
DELETE FROM reference_loot_template WHERE item IN(30283);

-- remove uses of now deleted ref template for above crafted items
DELETE FROM creature_loot_template WHERE mincountOrRef=-36100;

-- -----------
-- REPUTATION
-- -----------
-- Patch 2.1 - Coilfang Armaments will now drop from Heroic difficulty Slave Pens. - Creatures in the Serpentshrine now have a chance to drop Coilfang Armaments.
DELETE FROM creature_loot_template WHERE entry IN (21220,21218,21221,21224,21225,21226,21227,21228,21229,21230,21231,21232,21251,21263,21298,21299,21301,21339,21863,21127,21126,17957,17961,17959,17960,17958,17938) AND item = 24368;

-- ------------------
-- DUNGEONS AND RAIDS
-- -------------------
-- All 25 man raid bosses who drop set tokens will now drop an additional token in Patch 2.1.0 (Magtheridon and Gruul + HKM)
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 17257 AND item = 40400; -- Magtheridon
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 18831 AND item = 40300; -- High King Maulgar
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 19044 AND item = 40302; -- Gruul
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 19516 AND item = 36004; -- Void Reaver
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 19622 AND item = 36010; -- Kael'thas Sunstrider
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 21215 AND item = 36021; -- Leotheras
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 21214 AND item = 36023; -- Fathom-Lord Karathress
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 21212 AND item = 36026; -- Lady Vashj
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 17842 AND item = 36108; -- Azgalor
UPDATE creature_loot_template SET maxcount = 1 WHERE entry = 17968 AND item = 36107; -- Archimonde

-- ----------------------------------------------------------------------------
-- Epic gems did not drop in Heroic difficulty dungeons until 2.1
-- ----------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (40007,40019,40024,40037,40055,40064,40076,40085,40097,40104,40118,40125,40134,40145) AND mincountOrRef < 0;
DELETE FROM `gameobject_loot_template` WHERE `mincountOrRef` = -40037; -- Reinforced Fel Iron Chest 185169

-- ----------------------------------------------------------------------------------------
-- Epic item drops from Heroic difficulty dungeon end bosses were not 100% chance until 2.1
-- ----------------------------------------------------------------------------------------
UPDATE creature_loot_template SET ChanceOrQuestChance = 50 WHERE item IN (40006,40018,40023,40036,40046,40054,40063,40075,40084,40096,40103,40114,40117,40124,40133,40144) AND mincountOrRef < 0;
UPDATE `gameobject_loot_template` SET `ChanceOrQuestChance` = 50 WHERE `mincountOrRef` = -40036; -- Reinforced Fel Iron Chest 185169

-- --------------
-- FLIGHT MASTERS
-- --------------
UPDATE creature SET spawnMask=0 WHERE id IN (22935,22936,22931); -- Suralais Farwind, Auhula, Gorrim

-- Trial of the Naaru: Magtheridon

UPDATE quest_template SET RewItemId2=31704,RewItemCount2=1 WHERE entry IN(10888); -- give Tempest Key on completion
-- Add Tempest Key retrieval gossip to Ad'al
UPDATE areatrigger_teleport SET required_quest_done=10888 WHERE id IN(4470); -- require quest - fix from when conditions broke - just in case
DELETE FROM gossip_menu_option WHERE menu_id = 7966 AND id=0;
INSERT INTO gossip_menu_option(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) VALUES
(7966,0,0,'I have lost my Tempest Key.',1,1,-1,0,7966,0,0,'',20002);
DELETE FROM dbscripts_on_gossip WHERE id=7966 AND command=15;
INSERT INTO dbscripts_on_gossip(id, delay, command, datalong, datalong2, datalong3, buddy_entry, search_radius, data_flags, dataint, dataint2, dataint3, dataint4, x, y, z, o, comments) VALUES
(7966,0,15,39116,0,0,0,0,0,0,0,0,0,0,0,0,0,'Cast Create Tempest Key');
DELETE FROM conditions WHERE condition_entry BETWEEN 20000 AND 20002;
INSERT INTO conditions(`condition_entry`,`type`, `value1`, `value2`,`flags`,`comments`) VALUES
(20000,2,31704,1,1,''),
(20001,8,10888,0,0,''),
(20002,-1,20000,20001,0,'');

-- Mark of Ilidari drops from 2.1.0 onwards
DELETE FROM creature_loot_template WHERE item = 32897;
DELETE FROM reference_loot_template WHERE item = 32897;

-- -----------------------------------------
-- Magtheridon's Head did not drop until 2.1
-- -----------------------------------------
DELETE FROM creature_loot_template WHERE entry = 17257 AND item IN (32385,32386);

-- ----------------------------------------------------
-- Verdant Sphere did not drop from Kael'thas until 2.1
-- ----------------------------------------------------
DELETE FROM creature_loot_template WHERE entry = 19622 AND item = 32405;

-- Hyjal attunement items only dropped four per kill until 2.1
-- 19622 Kael'thas Sunstrider - 29905 Kael's Vial Remnant
-- 21212 Lady Vashj - 29906 Vashj's Vial Remnant
UPDATE item_template SET Flags=Flags&~2048 WHERE entry IN(29906,29905);
-- Kael - ref loot templates 90000-90003, Vashj - ref loot templates 90004-90007
DELETE FROM reference_loot_template WHERE entry IN (90000,90001,90002,90003);
DELETE FROM reference_loot_template WHERE entry IN (90004,90005,90006,90007);
INSERT INTO reference_loot_template (entry, item, ChanceOrQuestChance, groupid, mincountOrRef, maxcount, condition_id, comments) VALUES
(90000, 29905, -100, 0, 1, 1, 0, 'Kael''s Vial Remnant'),
(90001, 29905, -100, 0, 1, 1, 0, 'Kael''s Vial Remnant'),
(90002, 29905, -100, 0, 1, 1, 0, 'Kael''s Vial Remnant'),
(90003, 29905, -100, 0, 1, 1, 0, 'Kael''s Vial Remnant'),
(90004, 29906, -100, 0, 1, 1, 0, 'Vashj''s Vial Remnant'),
(90005, 29906, -100, 0, 1, 1, 0, 'Vashj''s Vial Remnant'),
(90006, 29906, -100, 0, 1, 1, 0, 'Vashj''s Vial Remnant'),
(90007, 29906, -100, 0, 1, 1, 0, 'Vashj''s Vial Remnant');
DELETE FROM creature_loot_template WHERE item IN (29906,29905);
INSERT INTO creature_loot_template (entry, item, ChanceOrQuestChance, groupid, mincountOrRef, maxcount, condition_id, comments) VALUES
(19622, 90000, 100, 0, -90000, 1, 0, 'Kael''s Vial Remnant'),
(19622, 90001, 100, 0, -90001, 1, 0, 'Kael''s Vial Remnant'),
(19622, 90002, 100, 0, -90002, 1, 0, 'Kael''s Vial Remnant'),
(19622, 90003, 100, 0, -90003, 1, 0, 'Kael''s Vial Remnant'),
(21212, 90004, 100, 0, -90004, 1, 0, 'Vashj''s Vial Remnant'),
(21212, 90005, 100, 0, -90005, 1, 0, 'Vashj''s Vial Remnant'),
(21212, 90006, 100, 0, -90006, 1, 0, 'Vashj''s Vial Remnant'),
(21212, 90007, 100, 0, -90007, 1, 0, 'Vashj''s Vial Remnant');

-- ------------------------------------------------------------------------------
-- Quest You, Robot (10248)
-- Prior to patch 2.1 the Scrap Reaver X6000 had "Repair" (34619) when controlled
-- ------------------------------------------------------------------------------
UPDATE creature_template_spells SET spell3=34619 WHERE entry=19849;

-- ---------------------------------------------------------------------------------------
-- Bottled Nethergon Energy/Vapor (Tempest Keep instance potions) were not added until 2.1
-- ---------------------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (32902,32905);

-- ------------------------------------------------
-- Level 70 Darkmoon Cards were not added until 2.1
-- ------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (49000,49001,49002) AND mincountorref IN (-49000,-49001,-49002);
DELETE FROM creature_loot_template WHERE item IN (31882,31892,31901,31910,31883,31884,31885,31886,31887,31888,31889,31893,31894,31895,31896,31898,31899,31900,31902,31903,31904,31905,31906,31908,31909,31911,31912,31913,31915,31916,31917,31918);
DELETE FROM `gameobject_loot_template` WHERE `mincountOrRef` = -49000; -- Reinforced Fel Iron Chest 185168/185169
DELETE FROM `reference_loot_template` WHERE `entry` = 49000; -- Darkmoon Cards (Blessings, Storms, Furies, Lunacy Ace) - Dungeon End Bosses (Levels: 70+)
UPDATE quest_template SET MinLevel=80 WHERE entry IN (10938,10939,10940,10941);
UPDATE item_template SET bonding=1 WHERE entry IN (31882,31892,31901,31910,31883,31884,31885,31886,31887,31888,31889,31893,31894,31895,31896,31898,31899,31900,31902,31903,31904,31905,31906,31908,31909,31911,31912,31913,31915,31916,31917,31918);

-- -----------------------------------------------------------------
-- Remove helmets added in 2.1 from G'eras (Badge of Justice vendor)
-- -----------------------------------------------------------------
DELETE FROM npc_vendor WHERE entry=18525 AND item IN (32083,32084,32085,32086,32087,32088,32089,32090);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (32083,32084,32085,32086,32087,32088,32089,32090);

-- From Patch 2.1 notes: "Primal Nethers may now be purchased from G'eras for Badges of Justice."
DELETE FROM npc_vendor WHERE entry=18525 AND item=23572;

-- --------------------------------------------------------------------
-- Gladiator's Gavel and Gladiator's Salvation were not added until 2.1
-- --------------------------------------------------------------------
DELETE FROM npc_vendor_template WHERE item IN (32450,32451);

-- ----------------------------------------------------------------------------------------------------------------------
-- Doomsday Candle (Warlock Dreadsteed questline) - Required 35 Black Dragonscales until it was reduced to 3 in Patch 2.1
-- ----------------------------------------------------------------------------------------------------------------------
UPDATE quest_template SET ReqItemCount1=35, Objectives='Bring 35 Black Dragonscales to Gorzeeki Wildeyes in the Burning Steppes.' WHERE entry=7628;

-- -----------------------------------------------------------------------
-- Kylene in Shattrath City is not a specialty cooking trainer until 2.1.2
-- -----------------------------------------------------------------------
DELETE FROM npc_trainer WHERE entry = 19186;
UPDATE creature_template SET NpcFlags = 643 WHERE entry = 19186;

-- ------------------------------------------------------------
-- Hellfire Shot and Felbane Slugs were not available until 2.1
-- ------------------------------------------------------------
DELETE FROM npc_vendor WHERE item IN (32882,32883);
DELETE FROM npc_vendor_template WHERE item IN (32882,32883);
UPDATE item_template SET ItemLevel=1, RequiredLevel=80 WHERE entry IN (32882,32883);

-- ----------------------
-- Netherstorm reversions
-- ----------------------
-- remove Ethereum Avengers/Nullifiers (need to double check this, but they shouldn't be stacked on the Shaleskin Ripper spawns)
UPDATE creature SET spawnMask=0 WHERE id IN (22821,22822);

-- add Shaleskin Ripper spawns which were present until 2.1
DELETE FROM creature WHERE id=20606;
INSERT INTO creature (guid, id, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecsmin, spawntimesecsmax, spawndist, currentwaypoint, curhealth, curmana, DeathState, MovementType) VALUES
(72707, 20606, 530, 1, 0, 0, 4205.4, 2398.72, 113.881, 4.77493, 300, 300, 5, 0, 0, 0, 0, 1),
(72708, 20606, 530, 1, 0, 0, 4229.99, 2447.66, 97.2873, 0.202567, 300, 300, 5, 0, 0, 0, 0, 1),
(72709, 20606, 530, 1, 0, 0, 4266.41, 2424.06, 105.908, 4.97111, 300, 300, 5, 0, 0, 0, 0, 1),
(72710, 20606, 530, 1, 0, 0, 4229.15, 2345.32, 139.556, 1.69709, 300, 300, 5, 0, 0, 0, 0, 1),
(72711, 20606, 530, 1, 0, 0, 4266.51, 2367.4, 122.204, 3.67513, 300, 300, 5, 0, 0, 0, 0, 1),
(72712, 20606, 530, 1, 0, 0, 4160.74, 2233.29, 172.03, 4.19894, 300, 300, 5, 0, 0, 0, 0, 1),
(72713, 20606, 530, 1, 0, 0, 4305.4, 2071.18, 133.879, 4.13548, 300, 300, 5, 0, 0, 0, 0, 1),
(72714, 20606, 530, 1, 0, 0, 4222.15, 1994.99, 141.751, 2.4683, 300, 300, 5, 0, 0, 0, 0, 1),
(72715, 20606, 530, 1, 0, 0, 4229.39, 1928.22, 144.377, 2.74959, 300, 300, 5, 0, 0, 0, 0, 1),
(72716, 20606, 530, 1, 0, 0, 4318.7, 1906.93, 122.415, 3.58581, 300, 300, 5, 0, 0, 0, 0, 1),
(72717, 20606, 530, 1, 0, 0, 4303.45, 1880.1, 128.679, 5.93948, 300, 300, 5, 0, 0, 0, 0, 1),
(72718, 20606, 530, 1, 0, 0, 4254.42, 1823.35, 141.345, 2.56902, 300, 300, 5, 0, 0, 0, 0, 1),
(72719, 20606, 530, 1, 0, 0, 4246.06, 1796.04, 133.143, 0.540953, 300, 300, 5, 0, 0, 0, 0, 1),
(72720, 20606, 530, 1, 0, 0, 4188.82, 1847.42, 150.792, 0.087511, 300, 300, 5, 0, 0, 0, 0, 1),
(72721, 20606, 530, 1, 0, 0, 4301.39, 1842.99, 123.467, 4.83664, 300, 300, 5, 0, 0, 0, 0, 1),
(72722, 20606, 530, 1, 0, 0, 4309.25, 1767.49, 115.554, 1.35117, 300, 300, 5, 0, 0, 0, 0, 1),
(72723, 20606, 530, 1, 0, 0, 4337.21, 1800.07, 109.388, 4.72801, 300, 300, 5, 0, 0, 0, 0, 1),
(72724, 20606, 530, 1, 0, 0, 4279.09, 1751.79, 116.245, 5.11534, 300, 300, 5, 0, 0, 0, 0, 1),
(72725, 20606, 530, 1, 0, 0, 4152.19, 1828.97, 154.204, 4.1718, 300, 300, 5, 0, 0, 0, 0, 1),
(72726, 20606, 530, 1, 0, 0, 4169.94, 1793.57, 140.599, 6.23045, 300, 300, 5, 0, 0, 0, 0, 1),
(72727, 20606, 530, 1, 0, 0, 4224.45, 1743.13, 124.89, 6.27422, 300, 300, 5, 0, 0, 0, 0, 1),
(72728, 20606, 530, 1, 0, 0, 4137.56, 1705.27, 124.936, 4.05002, 300, 300, 5, 0, 0, 0, 0, 1),
(72729, 20606, 530, 1, 0, 0, 4079.17, 1727.04, 141.987, 4.5238, 300, 300, 5, 0, 0, 0, 0, 1),
(72730, 20606, 530, 1, 0, 0, 4077.15, 1693.89, 136.815, 1.28765, 300, 300, 5, 0, 0, 0, 0, 1),
(72731, 20606, 530, 1, 0, 0, 4084.3, 1787.46, 148.901, 2.51637, 300, 300, 5, 0, 0, 0, 0, 1),
(72732, 20606, 530, 1, 0, 0, 3924.92, 1658.83, 129.177, 3.68871, 300, 300, 5, 0, 0, 0, 0, 1),
(72733, 20606, 530, 1, 0, 0, 3954.1, 1645.58, 124.275, 0.095156, 300, 300, 5, 0, 0, 0, 0, 1),
(72734, 20606, 530, 1, 0, 0, 3871.49, 1638.94, 126.29, 1.59909, 300, 300, 5, 0, 0, 0, 0, 1),
(72735, 20606, 530, 1, 0, 0, 3844.23, 1601.44, 122.755, 3.10681, 300, 300, 5, 0, 0, 0, 0, 1),
(72736, 20606, 530, 1, 0, 0, 3793.65, 1597.12, 126.04, 5.55176, 300, 300, 5, 0, 0, 0, 0, 1),
(72737, 20606, 530, 1, 0, 0, 3748.21, 1664.32, 137.335, 5.03215, 300, 300, 5, 0, 0, 0, 0, 1),
(72738, 20606, 530, 1, 0, 0, 3761.08, 1624.5, 132.833, 2.05715, 300, 300, 5, 0, 0, 0, 0, 1),
(72739, 20606, 530, 1, 0, 0, 3752.64, 1579.66, 118.341, 2.35605, 300, 300, 5, 0, 0, 0, 0, 1),
(72740, 20606, 530, 1, 0, 0, 3756.34, 1573.39, 116.359, 6.06176, 300, 300, 5, 0, 0, 0, 0, 1),
(72741, 20606, 530, 1, 0, 0, 3712.41, 1637.04, 127.955, 0.003011, 300, 300, 5, 0, 0, 0, 0, 1),
(72742, 20606, 530, 1, 0, 0, 3727.74, 1688.51, 134.894, 5.12689, 300, 300, 5, 0, 0, 0, 0, 1),
(72743, 20606, 530, 1, 0, 0, 3744.8, 1708.67, 143.519, 4.11829, 300, 300, 5, 0, 0, 0, 0, 1),
(72744, 20606, 530, 1, 0, 0, 3736, 1742.76, 143.207, 5.77488, 300, 300, 5, 0, 0, 0, 0, 1),
(72745, 20606, 530, 1, 0, 0, 3697.92, 1730.96, 130.4, 4.07045, 300, 300, 5, 0, 0, 0, 0, 1),
(72746, 20606, 530, 1, 0, 0, 3648.18, 1754.9, 120.197, 2.48012, 300, 300, 5, 0, 0, 0, 0, 1),
(72747, 20606, 530, 1, 0, 0, 3665.68, 1787.12, 126.969, 0.58096, 300, 300, 5, 0, 0, 0, 0, 1),
(72748, 20606, 530, 1, 0, 0, 3602.34, 1800.64, 108.563, 5.49088, 300, 300, 5, 0, 0, 0, 0, 1),
(72749, 20606, 530, 1, 0, 0, 3568.06, 1811.15, 108.413, 3.98777, 300, 300, 5, 0, 0, 0, 0, 1),
(72750, 20606, 530, 1, 0, 0, 3619.55, 1857.36, 114.978, 4.9319, 300, 300, 5, 0, 0, 0, 0, 1),
(72751, 20606, 530, 1, 0, 0, 3507.14, 1795.54, 84.899, 4.55585, 300, 300, 5, 0, 0, 0, 0, 1),
(72752, 20606, 530, 1, 0, 0, 3549.6, 1902.95, 95.3429, 3.25516, 300, 300, 5, 0, 0, 0, 0, 1),
(72753, 20606, 530, 1, 0, 0, 3599.22, 1912.46, 108.573, 4.19675, 300, 300, 5, 0, 0, 0, 0, 1),
(72754, 20606, 530, 1, 0, 0, 3577.47, 1993.15, 98.9251, 3.81557, 300, 300, 5, 0, 0, 0, 0, 1),
(72755, 20606, 530, 1, 0, 0, 3590.14, 1971.91, 105.183, 3.18848, 300, 300, 5, 0, 0, 0, 0, 1),
(72756, 20606, 530, 1, 0, 0, 3582.36, 2010.55, 94.0147, 1.99427, 300, 300, 5, 0, 0, 0, 0, 1),
(72757, 20606, 530, 1, 0, 0, 3596.5, 2043.42, 102.156, 4.21467, 300, 300, 5, 0, 0, 0, 0, 1),
(72758, 20606, 530, 1, 0, 0, 3613.99, 2031.07, 107.515, 5.42941, 300, 300, 5, 0, 0, 0, 0, 1),
(72759, 20606, 530, 1, 0, 0, 3644.61, 2142.95, 118.073, 5.92592, 300, 300, 5, 0, 0, 0, 0, 1),
(72760, 20606, 530, 1, 0, 0, 3573.72, 2227.43, 91.9982, 2.29817, 300, 300, 5, 0, 0, 0, 0, 1),
(72761, 20606, 530, 1, 0, 0, 3571.26, 2183.48, 88.1105, 2.48012, 300, 300, 5, 0, 0, 0, 0, 1),
(72762, 20606, 530, 1, 0, 0, 3627.53, 2244.61, 100.517, 5.5413, 300, 300, 5, 0, 0, 0, 0, 1),
(72763, 20606, 530, 1, 0, 0, 3610.75, 2228.07, 101.606, 0.90803, 300, 300, 5, 0, 0, 0, 0, 1),
(72764, 20606, 530, 1, 0, 0, 3631.3, 2209.06, 110.481, 1.21802, 300, 300, 5, 0, 0, 0, 0, 1),
(72765, 20606, 530, 1, 0, 0, 3676.18, 2234.16, 118.718, 1.49608, 300, 300, 5, 0, 0, 0, 0, 1),
(72766, 20606, 530, 1, 0, 0, 3587.99, 2261.34, 85.2089, 2.84331, 300, 300, 5, 0, 0, 0, 0, 1),
(72767, 20606, 530, 1, 0, 0, 3573.96, 2300.09, 74.9656, 4.71227, 300, 300, 5, 0, 0, 0, 0, 1),
(72768, 20606, 530, 1, 0, 0, 3636.38, 2305.56, 93.3712, 1.61963, 300, 300, 5, 0, 0, 0, 0, 1),
(72769, 20606, 530, 1, 0, 0, 3661.63, 2278.64, 106.82, 5.98011, 300, 300, 5, 0, 0, 0, 0, 1),
(72770, 20606, 530, 1, 0, 0, 3606.99, 2286.84, 82.3051, 1.96581, 300, 300, 5, 0, 0, 0, 0, 1),
(72771, 20606, 530, 1, 0, 0, 3718.23, 2257.58, 122.993, 4.9428, 300, 300, 5, 0, 0, 0, 0, 1),
(72772, 20606, 530, 1, 0, 0, 3723.16, 2218.22, 128.711, 4.11387, 300, 300, 5, 0, 0, 0, 0, 1),
(72773, 20606, 530, 1, 0, 0, 3597.84, 2316.69, 83.8905, 0.612646, 300, 300, 5, 0, 0, 0, 0, 1),
(72774, 20606, 530, 1, 0, 0, 3631.7, 2342.07, 90.7016, 1.22916, 300, 300, 5, 0, 0, 0, 0, 1),
(72775, 20606, 530, 1, 0, 0, 3703.6, 2273.78, 118.32, 5.09049, 300, 300, 5, 0, 0, 0, 0, 1),
(72776, 20606, 530, 1, 0, 0, 3636.07, 2398.65, 79.9635, 2.81644, 300, 300, 5, 0, 0, 0, 0, 1),
(72777, 20606, 530, 1, 0, 0, 3653.89, 2374.77, 87.5222, 4.0767, 300, 300, 5, 0, 0, 0, 0, 1),
(72778, 20606, 530, 1, 0, 0, 4129.52, 2196.25, 179.935, 2.48491, 300, 300, 5, 0, 0, 0, 0, 1),
(72779, 20606, 530, 1, 0, 0, 4151.57, 2179.44, 167.505, 1.68281, 300, 300, 5, 0, 0, 0, 0, 1);

-- =====================
-- Misc Raid Adjustments
-- =====================

-- Patch 2.1 - Smoking Blast on Nightbane deals less damage
UPDATE spell_template SET EffectDieSides1=691,EffectBasePoints1=4254 WHERE Id IN(30128);

-- Patch 2.1 - Roar is now susceptible to Horror effects.
UPDATE `creature_template` SET `MechanicImmuneMask` = `MechanicImmuneMask`|8388608 WHERE `entry` = 17546; -- MECHANIC_HORROR

-- Skeletal Usher shackles immunity (pre 2.1 only)
UPDATE `creature_template` SET `MechanicImmuneMask` = `MechanicImmuneMask`|524288 WHERE `entry` = 16471;

-- Patch 2.0.10 (2007-03-06): Restless Skeletons no longer have immolation.
DELETE FROM creature_template_addon WHERE `entry` = '17261';
INSERT INTO creature_template_addon (entry, mount, bytes1, b2_0_sheath, b2_1_flags, emote, moveflags, auras) VALUES
(17261,0,0,0,0,0,0,'37059'); -- Immolation

-- Nightbane - Smoking Blast 37057 - http://wowwiki.wikia.com/wiki/Nightbane_(boss)?direction=next&oldid=714882 (~Pre 2.1)
UPDATE `spell_template` SET `EffectDieSides1` = 690, `EffectBasePoints1` = 4254 WHERE `Id` = 37057; -- Initial Damage Mitigated by Armor 4255 to 4945
UPDATE `spell_template` SET `EffectBasePoints1` = 464 WHERE `Id` = 30127; -- Dot 2790 / 18 seconds

-- Astral Armor - Curator 15691 Pre-nerf
-- change to physical school so it may affect boss (has immunity to Arcane)
UPDATE spell_template SET SchoolMask=1 WHERE id=29476;

-- Romulo - Daring - buff is 50% up from 35% from some later patch - http://wowwiki.wikia.com/wiki/Patch_2.1.0
UPDATE spell_template SET EffectBasePoints1=49,EffectBasePoints2=49 WHERE Id IN(30841);

-- Spectral Retainer 16410 - Pre-nerf? Mass Despell
-- https://wow.gamepedia.com/index.php?title=Spectral_Retainer&direction=prev&oldid=1642392
DELETE FROM creature_ai_scripts WHERE id=1641005;
INSERT INTO creature_ai_scripts (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1641005','16410','0','0','100','1025','13000','15000','22000','22000','11','32375','4','1','0','0','0','0','0','0','0','0','Spectral Retainer - Cast Mass Dispel');

-- increase duration of Mind Exhaustion to 180 seconds (Patch 2.0) 30secs (9) 2.4.3
UPDATE spell_template SET DurationIndex=25 WHERE id IN(44032);

-- Patch 2.0.10 "The creatures that lead up to Hydross the Unstable and creatures at the six pumping stations are now on a 2 hour respawn instead of 45 minutes."
UPDATE creature SET spawntimesecsmin=2700, spawntimesecsmax=2700 WHERE id IN (21339,21221,21246,21301,21218,21263,21220);

-- Patch 2.1 "Coilfang Priestesses may now be polymorphed."
-- also immune to Charm pre-2.1? Can't find any official patch note for this
UPDATE `creature_template` SET MechanicImmuneMask=MechanicImmuneMask|1|65536 WHERE `entry` = '21220';

-- Patch 2.1 "Serpentshrine Lurkers are now banishable and fearable, and no longer create mushrooms so quickly."
UPDATE `creature_template` SET MechanicImmuneMask=MechanicImmuneMask|16|131072|8388608 WHERE `entry` = '21863';
UPDATE creature_ai_scripts SET event_param3=7000, event_param4=10000 WHERE id=2186305; -- 16800 21000

-- Patch 2.1 "The Coilfang Frenzy now does increased melee damage, but Scalding Water damage has been decreased."
UPDATE spell_template SET EffectBasePoints1=999,EffectBasePoints2=999 WHERE Id = 37284; -- 499

-- Patch 2.1 "Leotheras the Blind's Chaos Blast radius has been significantly reduced, which will allow additional melee to damage Leotheras while he is in metamorphosis form."
-- 15 yd (index 18) instead of 8 yd (index 14)
UPDATE spell_template SET EffectRadiusIndex1=18 WHERE id=37674; -- 14

-- Patch 2.1 "Coilfang Priestess' Holy Fire spell now deals Holy damage, and the damage dealt by the spell has been lowered."
-- no point in changing the spell school really... just modify the damage
UPDATE spell_template SET EffectBasePoints1=3642, EffectBasePoints2=2387 WHERE id=38585; -- 2642, 1387

-- Patch 2.1 "Toxic Spores no longer deal damage upon impact."
UPDATE spell_template SET Effect1=2,EffectDieSides1=451,EffectBaseDice1=1,EffectBasePoints1=2774,EffectImplicitTargetA1=18,EffectImplicitTargetB1=31,EffectRadiusIndex1=8 WHERE Id IN(38574);

-- Patch 2.1 - Morogrim Tidewalker has been moved to a more central location in his room.
UPDATE creature SET position_x='351.9414', `position_y`='-723.1434', `position_z`='-13.7273', `orientation`='3.2655' WHERE id=21213;

-- Patch 2.1 Kael'thas Shock Barrier has 20k less absorb
UPDATE spell_template SET EffectBasePoints1=99999 WHERE Id IN(36815);

-- Patch 2.1 "Crystalcore Devastator no longer use Charged Arcane Explosion - http://www.wowhead.com/npc=20040/crystalcore-devastator#comments:id=83187"
DELETE FROM `creature_ai_scripts` WHERE id=2004003;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2004003','20040','0','0','100','1025','21000','25000','25000','30000','11','37106','0','0','0','0','0','0','0','0','0','0','Crystalcore Devastator - Cast Charged Arcane Explosion');

-- Patch 2.1 "Crystalcore Sentinels no longer Trample."
-- spell ID is guessed
DELETE FROM `creature_ai_scripts` WHERE id=2004105;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2004105','20041','0','0','100','1025','5000','8000','9000','13000','11','40488','17','0','0','0','0','0','0','0','0','0','Crystalcore Sentinel - Cast Trample');

-- Patch 2.1 "Astromancers and Astromancer Lords no longer use the Blast Wave ability..."
-- spell ID is guessed
DELETE FROM creature_ai_scripts WHERE id IN (2004605,2003304);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2004605','20046','0','0','100','1025','4700','10800','10900','21200','11','38712','17','0','0','0','0','0','0','0','0','0','Astromancer Lord - Cast Blast Wave'),
('2003304','20033','0','0','100','1025','4700','10800','10900','21200','11','38712','17','0','0','0','0','0','0','0','0','0','Astromancer - Cast Blast Wave');

-- Patch 2.1 "The damage over time and radius of the Star Scryer's Starfall spell has been reduced."
-- radius set to 13 yd, up from 8 yd
UPDATE spell_template SET EffectBasePoints1=2999, EffectRadiusIndex1=17 WHERE id=37124; -- 1899, 14

-- Patch 2.2, Set in Patch 2.1 File due to Tuning Changes happening in 2.1 (halfing advisor hp)
-- Resurrection - used by Kael'thas in phase 3 to revive advisor adds (Change happened 7051t_2_2_0_engb -> 7091t_2_2_0_engb)
-- In the initial version of this encounter, the advisors have the same max HP in both phase 1 and phase 3 (it's not doubled, so we remove the 100% health increase effect here)
UPDATE spell_template SET Effect2=0, EffectDieSides2=0, EffectBaseDice2=0, EffectBasePoints2=0, EffectImplicitTargetA2=0, EffectImplicitTargetB2=0, EffectRadiusIndex2=0, EffectApplyAuraName2=0 WHERE Id=36450;

-- ========================
-- Misc Dungeon Adjustments
-- ========================

-- Nascent Fel Orc - from wowhead comment: "Can Be Mind Control in Heroic as of 4/12/2007 Patch 2.0.12"
UPDATE `creature_template` SET `MechanicImmuneMask`='1' WHERE `Entry`='18612';

-- Patch 2.1 - Pandemonius HC - Dark Shell 38759 - http://wowwiki.wikia.com/wiki/Patch_2.1.0
UPDATE `spell_template` SET `DurationIndex` = 31 WHERE `Id` = 38759; -- 6(32) to 8 (31)

-- Patch 2.1 - Sethekk Oracle's Arcane Lightning damage reduced and it will be cast less frequently.
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1832802,1832803);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1832802','18328','0','0','100','1027','6100','12100','7200','13300','11','32690','1','0','0','0','0','0','0','0','0','0','Sethekk Oracle (Normal) - Cast Arcane Lightning'),
('1832803','18328','0','0','100','1029','1200','12100','5200','11300','11','38146','1','0','0','0','0','0','0','0','0','0','Sethekk Oracle (Heroic) - Cast Arcane Lightning');

-- Patch 2.1.0 - Lieutenant Drake's melee speed has been slowed slightly and damage reduced.
UPDATE `creature_template` SET MeleeBaseAttackTime=2000 WHERE entry IN (17848,20535);

-- Patch 2.1.0 - The Durnholde Mage's Polymorph spell will be cast less frequently.
DELETE FROM `creature_ai_scripts` WHERE `id` = 1893408;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1893408','18934','0','0','100','1031','8700','12250','10000','15000','11','13323','5','544','0','0','0','0','0','0','0','0','Durnholde Mage - Cast Polymorph');

-- Patch 2.1 - Temporus' Spell Reflection ability on Heroic difficulty now has a duration of 6 instead of 8 seconds.
UPDATE `spell_template` SET `DurationIndex` = 31 WHERE `Id` = 38592;

-- Add Prenerf Domination Ability for Coilfang Enchantress 17961 - http://www.wowhead.com/npc=17961/coilfang-enchantress#comments:id=29712
DELETE FROM `creature_ai_scripts` WHERE `id` = 1796107;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1796107','17961','0','0','100','1025','15000','45000','30000','60000','11','35280','5','512','0','0','0','0','0','0','0','0','Coilfang Enchantress - Cast Domination');

-- Add Prenerf Blizzard Ability for Coilfang Technician 17940 - http://www.wowhead.com/npc=17940/coilfang-technician#comments:id=41586
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1794001,1794002);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1794001','17940','0','0','100','1059','9400','20500','20500','34100','11','16005','4','512','11','15783','4','512','0','0','0','0','Coilfang Technician (Normal) - Cast Rain of Fire Or Blizzard'),
('1794002','17940','0','0','100','1061','9400','20500','20500','34100','11','39376','4','512','11','37671','4','512','0','0','0','0','Coilfang Technician (Heroic) - Cast Rain of Fire Or Blizzard');

-- Patch 2.1 - Rokmar the Crackler will now only apply Grievous Wound to his current target.
-- https://github.com/cmangos/tbc-db/commit/be9f365437864bffe2dc1155cbb2c2db440d6db5 same change was applied to Ensnaring Moss at some point
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1799103,1799104,1799105);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1799103','17991','0','0','100','1025','15300','37400','22900','32600','11','31948','4','544','0','0','0','0','0','0','0','0','Rokmar the Crackler - Cast Ensnaring Moss'),
('1799104','17991','0','0','100','1027','8400','15700','15700','30100','11','31956','4','544','0','0','0','0','0','0','0','0','Rokmar the Crackler (Normal) - Cast Grievous Wound'),
('1799105','17991','0','0','100','1029','8400','15700','15700','30100','11','38801','4','544','0','0','0','0','0','0','0','0','Rokmar the Crackler (Heroic) - Cast Grievous Wound');

-- Patch 2.1 - Wrathfin Myrmidon will now have a slight delay before using the Coral Cut ability on a target.
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1772601,1772602);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1772601','17726','9','0','100','1027','0','5','10600','20400','11','31410','1','0','0','0','0','0','0','0','0','0','Wrathfin Myrmidon (Normal) - Cast Coral Cut'),
('1772602','17726','9','0','100','1029','0','5','10600','20400','11','37973','1','0','0','0','0','0','0','0','0','0','Wrathfin Myrmidon (Heroic) - Cast Coral Cut');

-- Patch 2.1 - Added a 2 second cast time to Omor the Unscarred's Treacherous Aura and Treacherous Bane abilities. - Treacherous Aura 30695 & Bane of Treachery 37566
UPDATE `spell_template` SET `CastingTimeIndex` = 1 WHERE `Id` IN (30695, 37566); -- 5

-- Patch 2.1 - Bleeding Hollow Scryer's Fear extended the repeat cooldown so the ability will happen less frequently
DELETE FROM `creature_ai_scripts` WHERE `id` = 1747804;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1747804','17478','0','0','100','1025','6600','23500','12100','20800','11','30615','5','544','0','0','0','0','0','0','0','0','Bleeding Hollow Scryer - Cast Fear');

-- Patch 2.1 - Laughing Skull Legionnaire no longer uses the Sweeping Strikes ability.
DELETE FROM `creature_ai_scripts` WHERE `id` = 1762603;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1762603','17626','0','0','100','1025','0','10000','30000','35000','11','18765','0','0','0','0','0','0','0','0','0','0','Laughing Skull Legionnaire - Cast Sweeping Strikes');

-- Patch 2.1 - Shadowmoon Technician's Silence spell is used less frequently.
DELETE FROM `creature_ai_scripts` WHERE `id` = 1741405;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1741405','17414','0','0','100','1025','5400','11600','12150','20300','11','6726','16','544','0','0','0','0','0','0','0','0','Shadowmoon Technician - Cast Silence on Random Player Mana User');

-- Patch 2.1 - Blood Guard Porung had a Frightening Shout Ability - https://www.wowhead.com/npc=20923/blood-guard-porung#comments:id=41122
DELETE FROM `creature_ai_scripts` WHERE `id` IN (2092307);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2092307','20923','0','0','100','1029','8000','16000','24000','32000','11','19134','1','0','0','0','0','0','0','0','0','0','Blood Guard Porung - Cast Frightening Shout (Heroic)');

-- Patch 2.1 - The Sunseeker Gene-Splicer's Death & Decay will occur less frequently and the damage has been reduced on Heroic difficulty.
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1950701,1950702);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1950701','19507','0','0','100','1027','4800','15300','10000','20000','11','34642','4','513','0','0','0','0','0','0','0','0','Sunseeker Gene-Splicer (Normal) - Cast Death and Decay'),
('1950702','19507','0','0','100','1029','4800','15300','10000','20000','11','39347','4','513','0','0','0','0','0','0','0','0','Sunseeker Gene-Splicer (Heroic) - Cast Death and Decay');

-- Patch 2.1 - Tempest-Forge Peacekeepers now deal Physical melee damage instead of Arcane, and now longer perform the Arcane Blast ability on Heroic difficulty. http://www.wowhead.com/npc=18405/tempest-forge-peacekeeper#comments:id=49024:reply=4923
UPDATE `creature_template` SET `damageschool` = 6 WHERE `entry` IN (18405,21578);

-- Patch 2.1 - Alar fight reworked - meteor deals significantly less damage
UPDATE spell_template SET EffectBasePoints1=142499,EffectDieSides1=15001 WHERE id IN(35181); -- Dive Bomb damage shared and much much higher

-- Patch 2.1 - killing the Embers of Al'ar will now be both much more possible and more rewarding to the raid.
-- Remove Al'ar Health Damage on Ember of Al'ar Death
DELETE FROM `spell_script_target` WHERE `entry` = 41910; -- Ember Blast

-- Patch 2.1 - Searing Cinders have stacks
UPDATE spell_template SET StackAmount=4 WHERE Id IN(30127);

-- =============
-- Custom Change
-- =============

-- Water Elemental Totem, Spitfire Totem - health increase to 50k (custom, no evidence or patch note for this)(Use as pre 2.1)
UPDATE spell_template SET EffectBasePoints1=49999 WHERE id IN (38624,38236);

-- Mutate Horror 19865 - Cast Corrode Armor 34643 - Source: TrinityCore and seems right, but could not find it yet in sniff, maybe was removed at some point
UPDATE `creature_template` SET `AIName` = 'EventAI' WHERE `entry` = 19865;
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1986501);
INSERT INTO `creature_ai_scripts` (`id`, `creature_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action1_type`, `action1_param1`, `action1_param2`, `action1_param3`, `action2_type`, `action2_param1`, `action2_param2`, `action2_param3`, `action3_type`, `action3_param1`, `action3_param2`, `action3_param3`, `comment`) VALUES
(1986501, 19865, 0, 0, 100, 1025, 0, 10000, 10000, 20000, 0, 0, 11, 34643, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Mutate Horror - Cast Corrode Armor');

-- Unconfirmed, maybe prenerf Web Ability for Crypt Fiend 17897, Source: tbc-db
-- https://github.com/cmangos/tbc-db/commit/01ffcd49656be3c3c3a993d48e7ff889b1697f83 - removed it due to not appearing in sniff
-- https://wowwiki.fandom.com/wiki/Crypt_Fiend_(Hyjal)?direction=prev&oldid=1123930 - readd as prenerf mechanic
DELETE FROM `creature_ai_scripts` WHERE id=1789702;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1789702','17897','0','0','75','1025','5000','15000','15000','35000','0','0','11','28991','17','0','0','0','0','0','0','0','0','0','Crypt Fiend - Cast Web');

-- Unconfirmed, maybe prenerf Enrage for Shadowmoon Riding Hound, Source: tbc-db
DELETE FROM `creature_ai_scripts` WHERE id=2308304;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2308304','23083','0','0','100','1025','5000','25000','30000','30000','11','8599','0','1','1','-106','0','0','0','0','0','0','Shadowmoon Riding Hound - Cast Enrage at 30% HP');

-- Unconfirmed, Let Gronn-Priest 21350 - Cast Heal on Friendly Missing HP - Post Nerf only casts it on himself after 50% HP
DELETE FROM `creature_ai_scripts` WHERE id=2135003;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2135003','21350','14','0','100','1025','118000','100','22000','28000','0','0','11','36678','12','0','0','0','0','0','0','0','0','0','Gronn-Priest - Cast Heal on Friendly Missing HP');

-- Unconfirmed, Let Lair Brute 19389 - Cast Charge and Reset Threat - more often
DELETE FROM `creature_ai_scripts` WHERE id=1938902;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1938902','19389','0','0','100','1025','8000','12000','15000','25000','0','0','11','24193','4','512','14','-100','0','0','0','0','0','0','Lair Brute - Cast Charge and Reset Threat');

-- Preventative measure to prevent players that already learned gem pattern from crafting it
UPDATE spell_template SET ReagentCount1=999999 WHERE Effect1=24 AND EffectItemType1 IN(31867,31863,31865,31868,32410,32409,31861);
-- More items that were added in 2.1 but we missed on launch. Don't allow crafting
UPDATE spell_template SET ReagentCount1=999999 WHERE Effect1=24 AND EffectItemType1 IN(32063,32067,32068,32062,32839,32849,32850,32851,32852,30034,30033,30032,30031,30040,30042,30046,30044,30043,30041,30045,
30039,30038,30036,30037,30035,32774,24053);

