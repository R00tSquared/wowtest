
-- This file should only be run BEFORE 2.3.0 is released

-- GetReputationPriceDiscount - mangosd.config:
-- DiscountReputationMod.Freindly = 0.05
-- DiscountReputationMod.Honored  = 0.10
-- DiscountReputationMod.Revered  = 0.15
-- DiscountReputationMod.Exalted  = 0.20

-- ------------------------------
-- Zul'Aman released in Patch 2.3
-- ------------------------------
UPDATE areatrigger_teleport SET required_level=80 WHERE id IN (4738);
UPDATE creature SET spawnMask=1 WHERE guid=180002; -- Spawn Raid Messenger (Zul'Aman)
-- Oooh, Shinies! quest
UPDATE quest_template SET MinLevel=80 WHERE entry=11130;

-- -------------------------------------------------
-- Ghostlands: Hatchet Hills (pre-Budd's expedition)
-- -------------------------------------------------
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=25145; -- Budd's Bodyguard
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=24851; -- Kiz Coilspanner <Flight Master>
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23747; -- Packhorse
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23724; -- Samir
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23745; -- Garg
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23559; -- Budd Nedreck
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23858; -- Donna Brascoe
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23748; -- Kurzel <Food & Drink>
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23560; -- Provisioner Ameenah <Reagents>
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23565; -- Turgore
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23718; -- Mack
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23766; -- Morgom
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23764; -- Marge
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23762; -- Brend
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=23761; -- Prigmon

-- we can at least assume that every corpse was once alive... re-visit this if we ever find pre-2.3 data
-- Moonwell: fix it?
-- `deathState`=0, 
-- `deathState`=0, 
UPDATE `creature` SET `MovementType`=1, `spawndist`=3, `id`=16345 WHERE `id`=23705; -- Catlord Corpse -> Shadowpine Catlord
UPDATE `creature` SET `MovementType`=1, `spawndist`=3, `id`=16346 WHERE `id`=23716; -- Hexxer Corpse -> Shadowpine Hexxer

UPDATE `gameobject` SET `spawnMask`=0 WHERE `id`=186302; -- Case of Orcish Grog
UPDATE `gameobject` SET `spawnMask`=0 WHERE `id`=186323; -- Burning Troll Hut
UPDATE `gameobject` SET `spawnMask`=0 WHERE `guid`=44723 AND `id`=186251; -- Meeting Stone

-- -------------------------------------------------------------------------------------------------------------
-- VENDOR DISCOUNTS WERE INTRODUCED IN 2.3 - REMOVE THE DISCOUNTS FROM THE CORE UNTIL REALM PROGRESSION HITS 2.3
-- -------------------------------------------------------------------------------------------------------------
-- EDIT CORE CONFIG FILE TO SET VENDOR DISCOUNTS - 0% UNTIL 2.3 THEN:
-- Friendly: 5% discount
-- Honored: 10% discount
-- Revered: 15% discount
-- Exalted: 20% discount

-- ===========================================
-- Pre 2.3.0 Badge of Justice Did Not Drop From Raid Bosses
-- ===========================================
DELETE FROM gameobject_loot_template WHERE item=29434 AND entry IN (20712); -- Karazhan - Dust Covered Chest
DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (16152,17521,17534,17533,18168,15687,16457,15691,15688,16524,15689,17225,15690); -- Karazhan - Bosses
DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (18831,19044); -- Gruul's Lair
DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (17257); -- Magtheridon's Lair
-- Zul'Aman Released in 2.3.0 /w Badge of Justice Drops

-- ----------------------------------------------------------------------------------
-- The standard Disarm ability that many creatures use now has a duration of 5 seconds instead of 6 seconds.
-- ----------------------------------------------------------------------------------
UPDATE `spell_template` SET `DurationIndex` = 32 WHERE `Id` = 6713; -- Disarm

-- ----------------------------------------------------------------------------------
-- Sonic Burst now silences for 6 seconds instead of 10.
-- ----------------------------------------------------------------------------------
UPDATE `spell_template` SET `DurationIndex` = 1 WHERE `Id` = 8281; -- Sonic Burst

-- -----------------------------------
-- Guild Vaults Were Introduced In 2.3 - Remove All Guild Vaults
-- -----------------------------------
UPDATE gameobject SET spawnMask=0 WHERE id IN (187334,187365,187294,187329,187299,187291,187292,187295,187337,187296,187290,187390,188127,188126);

-- ---------------------------------------------
-- Pre 2.3 Nerf Player-To-Level Experience Table
-- ---------------------------------------------
DELETE FROM player_xp_for_level;
INSERT INTO `player_xp_for_level` (`lvl`, `xp_for_next_level`) VALUES
('1','400'),('2','900'),('3','1400'),('4','2100'),('5','2800'),('6','3600'),('7','4500'),('8','5400'),('9','6500'),('10','7600'),
('11','8800'),('12','10100'),('13','11400'),('14','12900'),('15','14400'),('16','16000'),('17','17700'),('18','19400'),('19','21300'),
('20','23200'),('21','25200'),('22','27300'),('23','29400'),('24','31700'),('25','34000'),('26','36400'),('27','38900'),('28','41400'),('29','44300'),
('30','47400'),('31','50800'),('32','54500'),('33','58600'),('34','62800'),('35','67100'),('36','71600'),('37','76100'),('38','80800'),('39','85700'),
('40','90700'),('41','95800'),('42','101000'),('43','106300'),('44','111800'),('45','117500'),('46','123200'),('47','129100'),('48','135100'),('49','141200'),
('50','147500'),('51','153900'),('52','160400'),('53','167100'),('54','173900'),('55','180800'),('56','187900'),('57','195000'),('58','202300'),('59','209800'),
('60','494000'),('61','574700'),('62','614400'),('63','650300'),('64','682300'),('65','710200'),('66','734100'),('67','753700'),('68','768900'),('69','779700');

-- ---------------------------------------------------------
-- Flight Master in Rebel Camp STV - Was Not Added Until 2.3
-- ---------------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (24366);

-- --------------------------------------------------------------------------------------------
-- Tiny Sporebat Was Not Available Until 2.3 - Removed From Vendor Until Progression Unlocks It
-- --------------------------------------------------------------------------------------------
DELETE FROM npc_vendor WHERE item IN (34478);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (34478);

-- --------------------------------------------------------------------------------------------
-- Azure Whelpling Was Not Available Until 2.3 - Removed From Loot Until Progression Unlocks It
-- --------------------------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (34535);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (34535);

-- ---------------------------------------------------
-- Cenarion War Hippogryph was not available until 2.3
-- ---------------------------------------------------
DELETE FROM npc_vendor WHERE item = 33999;

-- --------------------------------------------------------------------------------------------
-- Formula: Enchant Ring - Stats was sold by Andormu before 2.3 instead of Nakodu and it required Scale of the Sands Revered instead of Lower City
-- https://web.archive.org/web/20071011042415/https://www.wowhead.com/?npc=20130
-- --------------------------------------------------------------------------------------------
DELETE FROM npc_vendor WHERE item=22538;
INSERT INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `condition_id`, `comments`) VALUES 
(19932, 22538, 0, 0, 0, 0, 'Formula: Enchant Ring - Stats');
UPDATE item_template SET RequiredReputationFaction=990, RequiredReputationRank=6 WHERE entry=22538;

-- ------------------------------------------------------------------------------
-- Schematic: Field Repair Bot 110G did not drop from Gan'arg Analyzers until 2.3
-- ------------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item = 34114;

-- -----------------------------------------------------------------------------
-- Schematic: Adamantite Arrow Maker did not drop from Sunfury Archers until 2.3
-- -----------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item = 33804;

-- -----------------------------------------------------------------------------
-- [Plans: Heavy Copper Longsword] was added as the new quest reward from q.1578 in 2.3
-- https://www.wowhead.com/item=33792/plans-heavy-copper-longsword#comments
-- https://web.archive.org/web/20071027073058/https://wowhead.com/?quest=1578
-- https://web.archive.org/web/20071208183503/http://www.wowhead.com/?quest=1578
-- -----------------------------------------------------------------------------
UPDATE quest_template SET RewItemId1=3609 WHERE entry=1578;

-- https://tbc.wowhead.com/quest=7861/wanted-vile-priestess-hexx-and-her-minions#comments:id=184066
-- 10 -> 20
UPDATE `quest_template` SET `Type`=1, `Objectives` = 'You have been ordered to slay Vile Priestess Hexx and 20 Vilebranch Aman\'zasi Guards. See Primal Torntusk at Revantusk Village in the Hinterlands once this task is complete.$B$BVile Priestess Hexx and the Aman\'zasi Guards can be found atop Jintha\'alor in the Hinterlands.', `ReqCreatureOrGOCount2` = '20' WHERE (`entry` = '7861');

-- https://tbc.wowhead.com/quest=7862/job-opening-guard-captain-of-revantusk-village#comments:id=2727108
-- 10	5	5	5 -> 20	20	20	20
UPDATE `quest_template` SET `Type`=1, `Objectives` = 'You have been tasked with the decimation of 20 Vilebranch Berserkers, 20 Vilebranch Shadow Hunters, 20 Vilebranch Blood Drinkers, and 20 Vilebranch Soul Eaters.$B$BShould you complete this task, return to Primal Torntusk at Revantusk Village in the Hinterlands.', `ReqCreatureOrGOCount1` = '20', `ReqCreatureOrGOCount2` = '20', `ReqCreatureOrGOCount3` = '20', `ReqCreatureOrGOCount4` = '20' WHERE (`entry` = '7862');

-- -------------------------------------------------
-- Glove Reinforcements were not available until 2.3
-- -------------------------------------------------
DELETE FROM npc_trainer_template WHERE spell = 44770;

-- ------------------------------------------------------
-- Heavy Knothide Armor Kits were not available until 2.3
-- ------------------------------------------------------
DELETE FROM npc_trainer_template WHERE spell = 44970;

-- -------------------------------------------------------------------------------
-- Pattern: Drums of Battle and Pattern: Drums of Panic required Exalted until 2.3 
-- -------------------------------------------------------------------------------
UPDATE item_template SET RequiredReputationRank = 7 WHERE entry IN (29717,29713);

-- -------------------------------------------------------------
-- Remove PvP and Normal/Heroic Dungeon and Cooking Daily Quests
-- -------------------------------------------------------------
-- Remove "Call to Arms" PvP Daily Quests (They were removed in 2.0.1 and Re-Added in 2.3)
UPDATE quest_template SET MinLevel=80 WHERE entry IN (11335,11336,11337,11338,11339,11340,11341,11342);

-- The Rokk Was Not In The Game (Along With Quests) Until 2.3 - Quest Giver NPC for Cooking Daily Quests
UPDATE creature SET spawnMask=0 WHERE id IN (24393);

-- Wind Trader Zhareem (Along With Quests) Until 2.3 - Quest Giver NPC for Normal/Heroic Dungeon Daily Quests
UPDATE creature SET spawnMask=0 WHERE id = 24369; -- Heroic Dungeon Daily Quest
UPDATE creature SET spawnMask=0 WHERE id = 24370; -- Normal Dungeon Daily Quest

-- ------------------------------------------------------------------------------
-- Pre 2.3.0: Sethekk Halls Normal Mode Only Awards Lower City Rep to Honored Max
-- ------------------------------------------------------------------------------
UPDATE creature_onkill_reputation SET MaxStanding1=4 WHERE MaxStanding1=5 AND creature_id IN (19429,18323,18318,18328,18327,21891,21904,19428,18325,18322,18326,18321,18319,18320);

-- --------------------------------------------------------------------------------
-- Pre 2.3.0: Sethekk Halls - Talon King Ikiss No Longer Drops Shadow Labyrinth Key - It is Located in The Talon King's Coffer Behind The Boss Now
-- --------------------------------------------------------------------------------
UPDATE gameobject SET spawnMask=0 WHERE id IN (187372);
DELETE FROM creature_loot_template WHERE entry IN (18473,20706) AND item IN (27991);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
('18473','27991','100','0','1','1','0','Shadow Labyrinth Key'), -- Normal Mode
('20706','27991','100','0','1','1','0','Shadow Labyrinth Key'); -- Heroic Mode

-- --------------------------------------------------------------------------------------------------------------------------
-- Pre 2.3.0: Mechanar - Heroic difficulty Gatewatcher Gyro-Kill and Gatewatcher Iron-Hand no longer drop [Badges of Justice]. Instead, the Cache of the Legion contains a Badge of Justice for each player present. 
-- --------------------------------------------------------------------------------------------------------------------------
-- Add Instance Bind to Gatewatcher Gyro-Kill & Gatewatcher Iron-Hand on Heroic Mode due to Badge of Justice Drop
UPDATE `creature_template` SET `ExtraFlags` = `ExtraFlags`|1 WHERE `entry` IN (21525,21526); -- CREATURE_EXTRA_FLAG_INSTANCE_BIND
DELETE FROM gameobject_loot_template WHERE entry IN (20530) AND item IN (29434);
DELETE FROM creature_loot_template WHERE entry IN (21525,21526) AND item IN (29434);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
('21525','29434','100','0','1','1','0','Badge of Justice'),
('21526','29434','100','0','1','1','0','Badge of Justice');

-- ---------------------------------------------------------
-- All Heroic Dungeons Required Revered Reputation (Pre 2.3) - Lowered to Honored in 2.3
-- ---------------------------------------------------------
UPDATE item_template SET RequiredReputationRank=6 WHERE entry IN (30622,30637,30633,30623,30634,30635);

-- ------------------------------------------------------------------
-- Netherscale Ammo Pouch, Knothide Quiver Was Not Unique Until 2.3.2
-- ------------------------------------------------------------------
UPDATE item_template SET flags=flags|524288 WHERE entry IN (34106,34100);

-- -----------------------------------------------------------
-- Vengeful Gladiator's Grimoire Was Not Available until 2.3.2
-- -----------------------------------------------------------
DELETE FROM npc_vendor_template WHERE item IN (34033);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (34033);

-- -------------------------------------------------------------------------
-- Plans: Adamantite Weapon Chain Was Not Available Until 2.3 as a Loot Drop
-- -------------------------------------------------------------------------
DELETE FROM reference_loot_template WHERE item IN (33186,35296);

-- -------------------------------------------------------------------------
-- Plans: Hammer of Righteous Might Was Not Available Until 2.3 as a Loot Drop
-- -------------------------------------------------------------------------
DELETE FROM reference_loot_template WHERE item IN (33954);

-- ------------------------------------------------------
-- Engineering Flying Mounts Were Not Available Until 2.3
-- ------------------------------------------------------
DELETE FROM npc_trainer WHERE spell IN (44155,44153,44157,44151);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (34060,34061);

-- Engineering trainers Jonathan Garrett (25099) and Niobe Whizzlespark (24868) likely didn't exist prior to 2.3
UPDATE creature SET spawnMask=0 WHERE id IN (25099,24868);

-- -------------------------------------------
-- Crashin' Thrashin' Robot was BoP Before 2.3
-- -------------------------------------------
UPDATE item_template SET bonding=1 WHERE entry IN (23767);

-- ----------------------------------------------------------------------------
-- Design: Chaotic Skyfire Diamond Started Dropping From Coilskar Sirens in 2.3
-- ----------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (34221,34689);

-- ----------------------------------------------------------------------------
-- Patch 2.3 - Shadowmoon Grunts will no longer drop Black Temple quality loot.
-- ----------------------------------------------------------------------------
DELETE FROM `creature_loot_template` WHERE `entry` = 23147 AND `item` IN (12005,32428,36198,36199);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
(23147, 12005, 6, 1, -12005, 1, 0, 'Epic Gem - TBC'),
(23147, 32428, 4, 0, 1, 1, 0, 'Heart of Darkness'),
(23147, 36198, 1, 1, -36198, 1, 0, 'Black Temple (Trash Loot) - Epic Items'),
(23147, 36199, 0.5, 1, -36199, 1, 0, 'Black Temple (Trash Loot) - Profession (-Pattern,-Plans)');

-- -----------------------------------------------------------------------------------------------------------------------------------------------------------
-- Leatherworker's Satchel (20 Slot Leatherworking Bag), Knothide Quiver and Knothide Ammo Pouch (20 Slot Ammo Bags) Was not Available From Trainers Until 2.3
-- -----------------------------------------------------------------------------------------------------------------------------------------------------------
DELETE FROM npc_trainer_template WHERE spell IN (45100,44343,44344);

-- -------------------------------------------
-- [Winter Boots]
-- Item ID: 34086, Recipe ID: 34262
-- http://wowwiki.wikia.com/Pattern:_Winter_Boots
-- "Patch 2.3.0 (13-Nov-2007): Added."
-- -------------------------------------------
DELETE FROM npc_vendor WHERE item=34262;
DELETE FROM npc_vendor_template WHERE item=34262;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(34086, 34262);

-- -------------------------------------------
-- [Leatherworker's Satchel]
-- Item ID: 34482, Spell ID: 45100
-- http://wowwiki.wikia.com/Patch_2.3.0
-- "A new recipe is available from Grand Master leatherworking trainers to make a 20 slot bag to hold leatherworking supplies."
-- -------------------------------------------
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(34482);
DELETE FROM npc_trainer_template WHERE spell IN (45100);
DELETE FROM npc_trainer WHERE spell IN (45100);

-- -------------------------------------------
-- [Knothide Ammo Pouch]
-- Item ID: 34099, Spell ID: 44343
-- [Knothide Quiver]
-- Item ID: 34100, Spell ID: 44344
-- http://wowwiki.wikia.com/Patch_2.3.0
-- "New recipes are available from Grand Master leatherworking trainers to make 20 slot quivers and ammo pouches."
-- -------------------------------------------
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, bonding=1 WHERE entry IN(34099, 34100);
DELETE FROM npc_trainer_template WHERE spell IN (44343, 44344);
DELETE FROM npc_trainer WHERE spell IN (44343, 44344);

-- -------------------------------------------
-- Weather-Beaten Journal
-- Added in 2.3
-- -------------------------------------------
-- DELETE FROM item_loot_template WHERE item=34109; -- Commented out as we have decided to keep this QoL addition

-- --------------------------------------------------------------------------------------------------------
-- Quiver of a Thousand Feathers and Netherscale Ammo Pouch (24 Slot Ammo Bags) Was Not Available Until 2.3
-- --------------------------------------------------------------------------------------------------------
DELETE FROM npc_vendor WHERE item IN (34200,34218,34201);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (34200,34218,34201);

-- -----------------------------------------------------
-- Pattern: Bag of Many Hides Was Not Released Until 2.3
-- -----------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (34491);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (34491);

-- ----------------------------------------------------------------------
-- Mana/Healing Potion Injectors Required Engineering To Use Prior to Patch 2.3.0
-- ----------------------------------------------------------------------
UPDATE item_template SET requiredskill = 202 WHERE entry = 33093; -- Mana Potion Injector (33093)
UPDATE item_template SET requiredskill = 202 WHERE entry = 33092; -- Healing Potion Injector (33092)

-- --------------------------------------------------
-- Mad Alchemist's Potion was not available until 2.3
-- --------------------------------------------------
DELETE FROM npc_trainer_template WHERE spell = 45061;

-- -------------------------------------------------------
-- Enchant Shield - Resilience was not available until 2.3
-- -------------------------------------------------------
DELETE FROM npc_trainer_template WHERE spell = 44383;

-- -----------------------------------------------------------------------------------------------------------------------------------------------
-- Pattern: Drums of Restoration and Pattern: Drums of Speed were world drops until 2.3 when they were added to Kurenai/Mag'har reputation vendors
-- -----------------------------------------------------------------------------------------------------------------------------------------------
DELETE FROM npc_vendor WHERE item IN (34172,34173,34174,34175);
-- Pattern: Drums of Speed, Pattern: Drums of Restoration 34172,34174 - Learn the same Spell
UPDATE `item_template` SET `RequiredReputationFaction` = 0, `RequiredReputationRank` = 0 WHERE `entry` IN (34172,34174);
-- 50501	NPC LOOT - Profession (-Design,-Formula,-Pattern,-Plans,-Recipe,-Schematic)(Non-BoP) - NPC Level 64+ Non-Elite/Level 58+ Elite - TBC NPC ONLY!
DELETE FROM `reference_loot_template` WHERE `entry` = 50501 AND `item` IN (34172,34174);
INSERT INTO `reference_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
(50501, 34172, 0, 1, 1, 1, 199, 'Pattern: Drums of Restoration'),
(50501, 34174, 0, 1, 1, 1, 199, 'Pattern: Drums of Speed');

-- ==========================================
-- Shattrath City Changes - Pre 2.3 Reversion
-- ==========================================

-- https://wowwiki.fandom.com/wiki/Dathris_Sunstriker
UPDATE `creature` SET `spawnmask` = 0 WHERE `id` = 18594; -- Dathris Sunstriker

-- -------------------------------------------------------------------------------------------
-- Amin <Apprentice Jewelcrafter> (Until patch 2.3, he was a journeyman jewelcrafting trainer)
-- -------------------------------------------------------------------------------------------
UPDATE creature_template SET NpcFlags = 17, GossipMenuId = 0 WHERE entry = 16703;
DELETE FROM npc_trainer WHERE entry IN (16703);
INSERT INTO `npc_trainer` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `condition_id`) VALUES
('16703','25229','10','0','0','5','0'),
('16703','25278','200','755','50','0','0'),
('16703','25280','200','755','50','0','0'),
('16703','25283','100','755','30','0','0'),
('16703','25284','400','755','60','0','0'),
('16703','25287','400','755','70','0','0'),
('16703','25490','300','755','50','0','0'),
('16703','26926','50','755','5','0','0'),
('16703','26927','300','755','50','0','0'),
('16703','26928','100','755','30','0','0'),
('16703','31252','100','755','20','5','0'),
('16703','32178','100','755','20','0','0'),
('16703','32179','100','755','20','0','0'),
('16703','32801','200','755','50','0','0');

-- --------------------------------------------
-- Pre 2.3 NPC Movement of Aldor in Center Area
-- --------------------------------------------
UPDATE creature SET SpawnDist=0, MovementType=2 WHERE guid IN (68464);
DELETE FROM creature_movement WHERE id IN (68464);
INSERT INTO `creature_movement` (`id`,`point`,`PositionX`,`PositionY`,`PositionZ`,`orientation`,`waittime`,`ScriptId`) VALUES 
(68464,1,-1879.79,5456.75,-12.4272,3.49232,0,0),
(68464,2,-1890.66,5445.78,-12.4272,4.04131,0,0),
(68464,3,-1896.59,5429.19,-12.4272,4.39788,0,0),
(68464,4,-1890.15,5409.25,-12.4272,5.08118,0,0),
(68464,5,-1872.55,5397.67,-12.4272,5.78175,0,0),
(68464,6,-1853.51,5397.42,-12.4272,0.007507,0,0),
(68464,7,-1836.46,5408.8,-12.4272,0.588702,0,0),
(68464,8,-1828.57,5427.83,-12.4272,1.17854,0,0),
(68464,9,-1833.26,5447.17,-12.4272,1.81,0,0),
(68464,10,-1846.49,5460.39,-12.4272,2.35742,0,0),
(68464,11,-1860.15,5463.07,-12.4272,3.01715,0,0);

-- -----------------------
-- Remove Misc Pre 2.3 NPC
-- -----------------------
UPDATE creature SET spawnMask=0 WHERE id IN (24727,24728); -- Caylee Dak and Dusky
UPDATE creature SET spawnMask=0 WHERE id=24729; -- Alicia

-- -------------------------------------------------------------------------
-- Remove 2.3 G'eras Vendor Items (Only Pre Kara Items Available Before 2.3)
-- -------------------------------------------------------------------------
DELETE FROM npc_vendor WHERE entry=18525 AND item IN (33192,33207,33222,33279,33280,33287,33291,33296,33304,33324,33325,33331,33333,33334,33386,33484,33501,33502,33503,33504,33505,33506,33507,33508,33509,33510,33512,33513,33514,33515,33516,33517,33518,33519,33520,33522,33523,33524,33527,33528,33529,33530,33531,33532,33534,33535,33536,33537,33538,33539,33540,33552,33557,33559,33566,33577,33578,33579,33580,33582,33583,33584,33585,33586,33587,33588,33589,33593,33810,33832,33965,33970,33972,33973,33974,34049,34050,34162,34163,35321,35324,35326);

-- ---------------------------------
-- Escape from skettis reward change
-- ---------------------------------
UPDATE quest_template SET rewchoiceitemID1 = 28100,  rewchoiceitemcount1 = 3, rewchoiceitemID2 = 28101, rewchoiceitemcount2 = 2, rewmoneymaxlevel = 57000,  Rewrepfaction1 = 1031, Rewrepvalue1 = 350, rewchoiceitemID2 = 28101, rewchoiceitemcount2 = 2 WHERE entry = 11085;

-- -------------------------------
-- Mysterious Arrows/Shells Added in 2.3
-- -------------------------------
DELETE FROM npc_vendor_template WHERE item IN (34581,34582);
DELETE FROM npc_vendor WHERE item IN (34581,34582);
UPDATE item_template SET ItemLevel=1, RequiredLevel=80 WHERE entry IN (34581,34582);

-- -------------------------------------
-- Cro Threadstrong had different texts on different patches
-- -------------------------------------
-- 2.2 texts guessed based on wowwiki and wowhead (http://wowwiki.wikia.com/wiki/Cro_Threadstrong) (http://www.wowhead.com/npc=19196/cro-threadstrong#comments)
DELETE FROM `dbscript_string` WHERE `entry` BETWEEN 2000001180 AND 2000001188;
INSERT INTO `dbscript_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`, `sound`, `type`, `language`, `emote`, `broadcast_text_id`, `comment`) VALUES 
(2000001180, 'Who is this fruit vendor to make such a bold move? He''s brought in an ogre for support.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 22057, 'Cro Threadstrong (Entry: 19196)'),
(2000001181, 'HA! The fruit vendor must be scared for his life.  He''s enlisted support.  Well two can play that game.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 22058, 'Cro Threadstrong (Entry: 19196)'),
(2000001182, 'Look at that!  Near the fruit carts, an ogre.  The fruit vendor is preparing for war...', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 22055, 'Cro Threadstrong (Entry: 19196)'),
(2000001183, 'Does the fruit vendor think that I am blind?  Do they think I wouldn''t see his ogre reinforcement?', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 22059, 'Cro Threadstrong (Entry: 19196)'),
(2000001184, 'That ogre is just the first sign that war is upon us.  The fruit vendor has gone too far.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 22056, 'Cro Threadstrong (Entry: 19196)'),

(2000001185, 'IF WAR IS WHAT YOU WANT, WAR IS WHAT YOU SHALL GET, FRUIT VENDOR!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 22064, 'Cro Threadstrong (Entry: 19196)'),
(2000001186, 'YOUR OGRE DOESN''T SCARE ME, FRUIT VENDOR! MY ARMY IS ON ITS WAY!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 22065, 'Cro Threadstrong (Entry: 19196)'),
(2000001187, 'YOUR DAYS ARE NUMBERED FRUIT VENDOR! I KNOW ABOUT YOUR OGRE ARMY!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 22066, 'Cro Threadstrong (Entry: 19196)'),
(2000001188, 'THE THOUSAND NATIONS OF THE ORC ARMY DESCEND UPON YOU FRUIT VENDOR. OUR ORC SHOULDERS SHALL BLOT OUT THE SUN!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, 53, 0, 'Cro Threadstrong (Entry: 19196)');

-- -------------------------------------------------
-- Shy-Rotam questline - Horde only before Patch 2.3
-- -------------------------------------------------
UPDATE quest_template SET RequiredRaces=690 WHERE entry IN ('5054', '5055', '5056');

-- -----------------------------------------------------------------------------------------------------------
-- CDB 0162 https://github.com/cmangos/classic-db/commit/e4e9918b4484158bcdbe30feef8f34b561dd4ac1
-- Reverted faction for creatures 2578 (Young Mesa Buzzard), 2579 (Mesa Buzzard), 2580 (Elder Mesa Buzzard)
-- as they were hostile to players prio to patch 2.3.2. Correct faction value is taken from other buzzard creatures
-- Source: http://http://www.wowwiki.com/Mesa_Buzzard
-- http://www.wowwiki.com/Elder_Mesa_Buzzard
-- http://www.wowwiki.com/Young_Mesa_Buzzard
-- -----------------------------------------------------------------------------------------------------------
UPDATE `creature_template` SET `Faction` = 73 WHERE `entry` IN (2578, 2579, 2580);

-- Slave Worker 5843 - http://www.wowhead.com/npc=5843/slave-worker#comments:id=202076
UPDATE `creature_template` SET `Faction` = 54 WHERE `entry` = 5843;

-- -------------------------------------------------
-- Npcs that were elite befor Patch 2.3
-- "Elite mobs outside of pre-Burning Crusade dungeons have been changed to non-elite."
-- "Many elite creatures and quests in the level 1-60 experience have been changed to accommodate solo play."
-- "Many NPCs in Stormpike and Frostwolf holds are no longer elite"
-- Patch 2.3.0 - The level ranges of pre-Burning Crusade dungeons have been adjusted to a narrower range.
-- Values taken from classic-db
-- https://www.wowhead.com/npc=314/eliza#comments:id=189741
UPDATE `creature_template` SET `Rank`='1', `MinLevel`='31', `MaxLevel`='31', `MeleeBaseAttackTime`='1341', `HealthMultiplier`='3', `Armor`='674' WHERE `entry`='314'; -- Eliza
-- http://wowwiki.wikia.com/wiki/Mor%27Ladim?direction=next&oldid=778724
UPDATE `creature_template` SET `MinLevel`='35', `MaxLevel`='35', `Armor`='1373' WHERE `entry`='522'; -- Mor'Ladim
-- http://wowwiki.wikia.com/wiki/Brainwashed_Noble?oldid=447524
UPDATE `creature_template` SET `Rank` = '2', `HealthMultiplier`='3', `MeleeBaseAttackTime`='1500' WHERE `entry`='596'; -- Brainwashed Noble
-- https://www.wowhead.com/npc=599/marisa-dupaige#comments:id=201047
UPDATE `creature_template` SET `Rank` = '2', `HealthMultiplier`='3' WHERE `entry`='599'; -- Marisa du'Paige
UPDATE `creature_template` SET `MaxLevel` = 20 WHERE `entry` = 636; -- https://classic.wowhead.com/npc=636/defias-blackguard
-- https://www.wowhead.com/npc=639/edwin-vancleef#comments:id=169282
UPDATE `creature_template` SET `MinLevel`='21', `MaxLevel`='21' WHERE `entry`='639'; -- Edwin VanCleef
-- https://www.wowhead.com/npc=680/moshogg-lord#comments:id=186056
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `MeleeBaseAttackTime`='1258' WHERE `entry`='680'; -- Mosh'Ogg Lord
-- https://www.wowhead.com/npc=723/moshogg-butcher#comments:id=1690511
UPDATE `creature_template` SET `Rank` = '2', `HealthMultiplier`='3' WHERE `entry`='723'; -- Mosh'Ogg Butcher
-- https://wow.gamepedia.com/index.php?title=Bhag%27thera&direction=next&oldid=776674
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3' WHERE `entry`='728'; -- Bhag'thera
-- https://wow.gamepedia.com/index.php?title=Tethis&direction=next&oldid=960753
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier` = '3.1' WHERE `entry`='730'; -- Tethis
-- https://www.wowhead.com/npc=813/colonel-kurzen#comments:id=184499
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1341' WHERE `entry`='813'; -- Colonel Kurzen
-- https://www.wowhead.com/npc=818/maizoth#comments:id=184522
UPDATE `creature_template` SET `Rank` = '1', `MinLevel`='47', `MaxLevel`='47', `HealthMultiplier`='4.29', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1258' WHERE `entry`='818'; -- Mai'Zoth
-- -- https://www.wowhead.com/npc=871/saltscale-warrior#comments:id=185874
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1341', `ArmorMultiplier`='1' WHERE `entry`='871'; -- Saltscale Warrior
-- https://www.wowhead.com/npc=873/saltscale-oracle#comments:id=185878
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1341', `ArmorMultiplier`='1' WHERE `entry`='873'; -- Saltscale Oracle
-- https://www.wowhead.com/npc=875/saltscale-tide-lord#comments:id=185879
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1341' WHERE `entry`='875'; -- Saltscale Tide Lord
-- https://www.wowhead.com/npc=877/saltscale-forager#comments:id=185872
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1341', `ArmorMultiplier`='1' WHERE `entry`='877'; -- Saltscale Forager
-- https://www.wowhead.com/npc=879/saltscale-hunter#comments:id=185876
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier` = '3', `PowerMultiplier` = '2', `MeleeBaseAttackTime`='1341', `ArmorMultiplier`='1' WHERE `entry`='879'; -- Saltscale Hunter
-- https://www.wowhead.com/npc=1225/ol-sooty#comments:id=185662
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier`='3' WHERE `entry`='1225'; -- Ol' Sooty
-- https://wowwiki.fandom.com/wiki/Dextren_Ward?oldid=1479007
UPDATE `creature_template` SET `MinLevel`='26', `MaxLevel`='26', `DamageMultiplier`='2', `MeleeBaseAttackTime`='2000' WHERE `entry`='1663'; -- Dextren Ward
-- https://wowwiki.fandom.com/wiki/Bazil_Thredd?direction=prev&oldid=1037707
UPDATE `creature_template` SET `MinLevel`='29', `MaxLevel`='29' WHERE `entry`='1716'; -- Bazil Thredd
-- https://wowwiki.fandom.com/wiki/Hamhock?direction=prev&oldid=1135173
UPDATE `creature_template` SET `MinLevel`='28', `MaxLevel`='28' WHERE `entry`='1717'; -- Hamhock
-- https://wowwiki.fandom.com/wiki/Bruegal_Ironknuckle?oldid=1599614
UPDATE `creature_template` SET `MinLevel`='26', `MaxLevel`='26' WHERE `entry`='1720'; -- Bruegal Ironknuckle
-- https://wow.gamepedia.com/index.php?title=Scarlet_Judge&direction=prev&oldid=402543
UPDATE `creature_template` SET `Rank`='2', `MeleeBaseAttackTime`='1150' WHERE `entry`='1837'; -- Scarlet Judge
UPDATE `creature_loot_template` SET `mincountOrRef` = -34010 WHERE `mincountOrRef` = -60294 AND `entry` = 1837; -- https://www.wowhead.com/npc=1837/scarlet-judge#comments:id=709160
-- https://www.wowhead.com/npc=1885/scarlet-smith#comments:id=204148
UPDATE `creature_template` SET `Faction`='67', `Rank`='2', `MinLevel`='59', `ArmorMultiplier`='1.5' WHERE `entry`='1885'; -- Scarlet Smith
-- https://de.wowhead.com/npc=2257/mugthol#comments:id=162682:reply=316694
UPDATE `creature_template` SET `Rank`='1', `MeleeBaseAttackTime`='1258', `HealthMultiplier`='3' WHERE `entry`='2257'; -- Mug'thol
-- https://www.wowhead.com/npc=2287/crushridge-warmonger#comments:id=184716
UPDATE `creature_template` SET `Rank`='1', `MeleeBaseAttackTime`='1341', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='2287'; -- Crushridge Warmonger
-- https://www.wowhead.com/npc=2635/elder-snapjaw-crocolisk#comments:id=184829
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='2635'; -- Elder Saltwater Crocolisk
-- https://wowwiki.fandom.com/wiki/Archaedas
UPDATE `creature_template` SET `MinLevel`='47', `MaxLevel`='47' WHERE `entry`='2748'; -- Archaedas
-- https://wowwiki.fandom.com/wiki/Mutanus_the_Devourer?direction=prev&oldid=926276
UPDATE `creature_template` SET `MinLevel`='22', `MaxLevel`='22' WHERE `entry`='3654'; -- Mutanus the Devourer
-- https://wowwiki.fandom.com/wiki/Lord_Pythas?direction=prev&oldid=926273
UPDATE `creature_template` SET `MinLevel`='21', `MaxLevel`='21' WHERE `entry`='3670'; -- Lord Pythas
-- https://wowwiki.fandom.com/wiki/Lord_Serpentis?direction=prev&oldid=926169
UPDATE `creature_template` SET `MinLevel`='21', `MaxLevel`='21' WHERE `entry`='3673'; -- Lord Serpentis
-- https://wowwiki.fandom.com/wiki/Skum?direction=prev&oldid=926277
UPDATE `creature_template` SET `MinLevel`='21', `MaxLevel`='21' WHERE `entry`='3674'; -- Skum
-- https://wowwiki.fandom.com/wiki/Razorclaw_the_Butcher?direction=prev&oldid=1124016
UPDATE `creature_template` SET `MinLevel`='22', `MaxLevel`='22' WHERE `entry`='3886'; -- Razorclaw the Butcher
-- https://wowwiki.fandom.com/wiki/Baron_Silverlaine?direction=prev&oldid=1300022
UPDATE `creature_template` SET `MinLevel`='24', `MaxLevel`='24' WHERE `entry`='3887'; -- Baron Silverlaine
-- https://wowwiki.fandom.com/wiki/Wolf_Master_Nandos?direction=prev&oldid=1124001
UPDATE `creature_template` SET `MinLevel`='25', `MaxLevel`='25' WHERE `entry`='3927'; -- Wolf Master Nandos
-- https://www.wowhead.com/npc=3975/herod#comments:id=188734
UPDATE `creature_template` SET `MinLevel`='40', `MaxLevel`='40' WHERE `entry`='3975'; -- Herod
-- https://www.wowhead.com/npc=3976/scarlet-commander-mograine#comments:id=183892
UPDATE `creature_template` SET `MinLevel`='42', `MaxLevel`='42' WHERE `entry`='3976'; -- Scarlet Commander Mograine
-- https://www.wowhead.com/npc=3977/high-inquisitor-whitemane#comments:id=188737
UPDATE `creature_template` SET `MinLevel`='42', `MaxLevel`='42' WHERE `entry`='3977'; -- High Inquisitor Whitemane
-- https://wowwiki.fandom.com/wiki/Fenrus_the_Devourer?oldid=1110419
UPDATE `creature_template` SET `MinLevel`='25', `MaxLevel`='25' WHERE `entry`='4274'; -- Fenrus the Devourer
-- https://wowwiki.fandom.com/wiki/Archmage_Arugal?direction=prev&oldid=980419
UPDATE `creature_template` SET `MinLevel`='26', `MaxLevel`='26' WHERE `entry`='4275'; -- Archmage Arugal
-- https://wowwiki.fandom.com/wiki/Commander_Springvale?direction=prev&oldid=1124011
UPDATE `creature_template` SET `MinLevel`='24', `MaxLevel`='24' WHERE `entry`='4278'; -- Commander Springvale
-- https://wowwiki.fandom.com/wiki/Odo_the_Blindwatcher?direction=prev&oldid=1124010
UPDATE `creature_template` SET `MinLevel`='24', `MaxLevel`='24' WHERE `entry`='4279'; -- Odo the Blindwatcher
-- https://wowwiki.fandom.com/wiki/Overlord_Ramtusk - https://www.wowhead.com/npc=4420/overlord-ramtusk#comments:id=183912
UPDATE `creature_template` SET `MinLevel`='32', `MaxLevel`='32' WHERE `entry`='4420'; -- Overlord Ramtusk
-- https://www.wowhead.com/npc=4421/charlga-razorflank#comments:id=102086 - http://wowwiki.wikia.com/wiki/Charlga_Razorflank?direction=next&oldid=362313
UPDATE `creature_template` SET `MinLevel` = '33', `MaxLevel` = '33', `Armor`='1091' WHERE `entry`='4421'; -- Charlga Razorflank
-- https://wowwiki.fandom.com/wiki/Agathelos_the_Raging?direction=prev&oldid=1016548
UPDATE `creature_template` SET `MinLevel`='33', `MaxLevel`='33', `PowerMultiplier`='2' WHERE `entry`='4422'; -- Agathelos the Raging
-- https://wowwiki.fandom.com/wiki/Aggem_Thorncurse?direction=prev&oldid=1016534
UPDATE `creature_template` SET `MinLevel`='30', `MaxLevel`='30' WHERE `entry`='4424'; -- Aggem Thorncurse
-- https://wowwiki.fandom.com/wiki/Blind_Hunter?direction=prev&oldid=1108073
UPDATE `creature_template` SET `MinLevel`='32', `MaxLevel`='32' WHERE `entry`='4425'; -- Blind Hunter
-- https://wowwiki.fandom.com/wiki/Death_Speaker_Jargba?direction=prev&oldid=1016526
UPDATE `creature_template` SET `MinLevel`='30', `MaxLevel`='30' WHERE `entry`='4428'; -- Death Speaker Jargba
-- https://wowwiki.fandom.com/wiki/Bloodmage_Thalnos?direction=prev&oldid=985940
UPDATE `creature_template` SET `MinLevel`='34', `MaxLevel`='34' WHERE `entry`='4543'; -- Bloodmage Thalnos
-- https://wowwiki.fandom.com/wiki/Arugal%27s_Voidwalker?direction=next&oldid=1388166
UPDATE `creature_template` SET `MinLevel`='24', `MaxLevel`='25' WHERE `entry`='4627'; -- Arugal's Voidwalker
-- https://wowwiki.fandom.com/wiki/Aku%27mai_the_Devourer?direction=prev&oldid=928337 - says 29 but 28 is sniff and correct in classic-db
UPDATE `creature_template` SET `MinLevel`='28', `MaxLevel`='28' WHERE `entry`='4829'; -- Aku'mai
-- https://wowwiki.fandom.com/wiki/Old_Serra%27kis?direction=prev&oldid=928335
UPDATE `creature_template` SET `MinLevel`='26', `MaxLevel`='26' WHERE `entry`='4830'; -- Old Serra'kis
-- https://wowwiki.fandom.com/wiki/Lady_Sarevess?direction=next&oldid=880566
UPDATE `creature_template` SET `MinLevel`='25', `MaxLevel`='25' WHERE `entry`='4831'; -- Lady Sarevess
-- https://wowwiki.fandom.com/wiki/Twilight_Lord_Kelris?direction=prev&oldid=928336
UPDATE `creature_template` SET `MinLevel`='27', `MaxLevel`='27' WHERE `entry`='4832'; -- Twilight Lord Kelris
-- https://wowwiki.fandom.com/wiki/Earthcaller_Halmgar?direction=prev&oldid=1017596
UPDATE `creature_template` SET `MinLevel`='32', `MaxLevel`='32' WHERE `entry`='4842'; -- Earthcaller Halmgar
-- https://wowwiki.fandom.com/wiki/Grimlok?direction=prev&oldid=1034819 - https://www.wowhead.com/npc=4854/grimlok#comments:id=232445
UPDATE `creature_template` SET `MinLevel`='45', `MaxLevel`='45' WHERE `entry`='4854'; -- Grimlok
-- https://wowwiki.fandom.com/wiki/Ghamoo-ra?direction=prev&oldid=1194585
UPDATE `creature_template` SET `MinLevel`='25', `MaxLevel`='25', `ArmorMultiplier`='1000' WHERE `entry`='4887'; -- Ghamoo-ra
-- https://wowpedia.fandom.com/wiki/Jammal%27an_the_Prophet?oldid=1559394
UPDATE `creature_template` SET `MinLevel`='54', `MaxLevel`='54' WHERE `entry`='5710'; -- Jammal'an the Prophet
-- https://wowpedia.fandom.com/wiki/Ogom_the_Wretched?oldid=1479996
UPDATE `creature_template` SET `MinLevel`='53', `MaxLevel`='53' WHERE `entry`='5711'; -- Ogom the Wretched
-- http://www.wowhead.com/npc=6231/techbot#comments:id=184452
UPDATE `creature_template` SET `Rank` = '1', `MeleeBaseAttackTime`='1425' WHERE `entry`='6231'; -- Techbot
-- https://wowwiki.fandom.com/wiki/Gelihast?oldid=1479998
UPDATE `creature_template` SET `MinLevel`='26', `MaxLevel`='26' WHERE `entry`='6243'; -- Gelihast
--
UPDATE `creature_template` SET `MinLevel`='48', `MaxLevel`='48', `PowerMultiplier`='2' WHERE `entry`='7267'; -- Chief Ukorz Sandscalp
-- https://www.wowhead.com/npc=7355/tutenkash#comments:id=183926 - http://wowwiki.wikia.com/wiki/Tuten%27kash?direction=prev&oldid=814679
UPDATE `creature_template` SET `MinLevel` = '40', `MaxLevel` = '40' WHERE `entry` = '7355'; -- Tuten'kash
-- https://wow.gamepedia.com/Amnennar_the_Coldbringer_(original)
UPDATE `creature_template` SET `MinLevel`='41', `MaxLevel`='41' WHERE `entry`='7358'; -- Amnennar the Coldbringer
--
UPDATE `creature_template` SET `MinLevel`='46', `MaxLevel`='46', `PowerMultiplier`='2' WHERE `entry`='7797'; -- Ruuzlu
-- https://www.wowhead.com/npc=7800/mekgineer-thermaplugg#comments:id=196852
UPDATE `creature_template` SET `MinLevel` = '34', `MaxLevel` = '34', `ArmorMultiplier`='2' WHERE `entry`='7800'; -- Mekgineer Thermaplugg
-- http://www.wowhead.com/npc=8444/trade-master-kovic#comments
UPDATE `creature_template` SET `Rank` = '1', `MinLevel`='50', `MaxLevel`='50' WHERE `entry`='8444'; -- Trade Master Kovic
-- http://www.wowhead.com/npc=8447/clunk#comments
UPDATE `creature_template` SET `Rank` = '1', `MinLevel`='48', `MaxLevel`='48', `HealthMultiplier`='3', `DamageMultiplier`='3.25', `MeleeBaseAttackTime`='1258' WHERE `entry`='8447'; -- Clunk
-- https://www.wowhead.com/npc=8924/the-behemoth#comments:id=180917:reply=20143
UPDATE `creature_template` SET `Rank` = '2', `HealthMultiplier`='5', `MeleeBaseAttackTime`='1233' WHERE `entry`='8924'; -- The Behemoth
-- http://wowwiki.wikia.com/wiki/Borelgore
UPDATE `quest_template` SET `Type`=1 WHERE `entry`=6136; -- The Corpulent One
UPDATE `creature_template` SET `Rank`='1', `DamageMultiplier`='5.5', `HealthMultiplier`='14.5' WHERE `entry`='11896'; -- Borelgore
-- http://wowwiki.wikia.com/wiki/Duskwing
UPDATE `quest_template` SET `Type`=1 WHERE `entry`=6135; -- Duskwing, Oh How I Hate Thee...
UPDATE `creature_template` SET `Rank`='1', `DamageMultiplier`='5', `HealthMultiplier`='6' WHERE `entry`='11897'; -- Duskwing
-- https://wowwiki.fandom.com/wiki/Baron_Aquanis?direction=prev&oldid=928463
UPDATE `creature_template` SET `MinLevel`='28', `MaxLevel`='28', `DamageMultiplier`='2' WHERE `entry`='12876'; -- Baron Aquanis
-- https://www.wowhead.com/npc=14621/overseer-maltorius#comments:id=184414
UPDATE `creature_template` SET `Rank` = '1', `HealthMultiplier`='7.7', `PowerMultiplier`='2', `MeleeBaseAttackTime`='1266' WHERE `entry`='14621'; -- Overseer Maltorius
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecsmin`, `spawntimesecsmax`, `spawndist`, `movementtype`) VALUES
(5848, 8504, 0, -6631.35, -1232.93, 209.808, 1.37082, 500, 500, 0, 0);

-- ========
-- Silithus
-- ========
-- Earth Elemental 329
UPDATE `creature_template` SET `MinLevel`='54', `MaxLevel`='55' WHERE `entry`='329'; -- Earth Elemental
-- Hive'Ashi Stinger 11698
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11698'; -- Hive'Ashi Stinger
-- Hive'Ashi Worker 11721
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11721'; -- Hive'Ashi Worker
-- Hive'Ashi Defender 11722
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1.5' WHERE `entry`='11722'; -- Hive'Ashi Defender
-- Hive'Ashi Sandstalker 11723
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='2.85' WHERE `entry`='11723'; -- Hive'Ashi Sandstalker
-- Hive'Ashi Swarmer 11724
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11724'; -- Hive'Ashi Swarmer
-- Hive'Zora Waywatcher 11725
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11725'; -- Hive'Zora Waywatcher
-- Hive'Zora Tunneler 11726
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11726'; -- Hive'Zora Tunneler
-- Hive'Zora Wasp 11727
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11727'; -- Hive'Zora Wasp
-- Hive'Zora Reaver 11728
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11728'; -- Hive'Zora Reaver
-- Hive'Zora Hive Sister 11729
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11729'; -- Hive'Zora Hive Sister
-- Hive'Regal Ambusher 11730
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='2.85' WHERE `entry`='11730'; -- Hive'Regal Ambusher
-- Hive'Regal Burrower 11731
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11731'; -- Hive'Regal Burrower
-- Hive'Regal Spitfire 11732
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11732'; -- Hive'Regal Spitfire
-- Hive'Regal Slavemaker 11733
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='3', `ArmorMultiplier`='1' WHERE `entry`='11733'; -- Hive'Regal Slavemaker
-- Hive'Regal Hive Lord 11734
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='4' WHERE `entry`='11734'; -- Hive'Regal Hive Lord
-- Stonelash Scorpid 11735
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11735'; -- Stonelash Scorpid
-- Stonelash Pincer 11736
-- Stonelash Flayer 11737
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11737'; -- Stonelash Flayer
-- Sand Skitterer 11738
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11738'; -- Sand Skitterer
-- Rock Stalker 11739
UPDATE `creature_template` SET `MaxLevel`='59', `ArmorMultiplier`='1' WHERE `entry`='11739'; -- Rock Stalker
-- Dredge Striker 11740
-- https://github.com/Atlantiss/NetherwingBugtracker/issues/1963
UPDATE `creature_template` SET `SkinningLootId` = 0 WHERE `entry` = 11740; -- Dredge Striker
-- Dredge Crusher 11741
UPDATE `creature_template` SET `SkinningLootId` = 0 WHERE `entry` = 11741; -- Dredge Crusher
-- Dust Stormer 11744
-- Cyclone Warrior 11745
-- Desert Rumbler 11746
UPDATE `creature_template` SET `ArmorMultiplier`='1.25' WHERE `entry`='11746'; -- Desert Rumbler
-- Desert Rager 11747
UPDATE `creature_template` SET `ArmorMultiplier`='1.25' WHERE `entry`='11747'; -- Desert Rager
-- Twilight Keeper Exeter 11803
-- Twilight Keeper Havunth 11804
-- Jarund Stoutstrider 11805
-- Twilight Avenger 11880
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11880'; -- Twilight Avenger
-- Twilight Geolord 11881
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11881'; -- Twilight Geolord
-- Twilight Stonecaller 11882
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='11882'; -- Twilight Stonecaller
-- Twilight Master 11883
-- Tortured Druid 12178
-- Tortured Sentinel 12179
-- Shade of Ambermoon 12199
-- Zannok Hidepiercer 12956
-- Hive'Ashi Drone 13136
-- Layo Starstrike 13220
-- Hive''Ashi Ambusher 13301
-- Highlord Demitrian 14347 - npc_highlord_demitrian
-- Prince Thunderaan 14435
UPDATE `creature_template` SET `PowerMultiplier`='100' WHERE `entry`='14435'; -- Prince Thunderaan
-- The Windreaver 14454
-- Whirling Invader 14455
-- Setis 14471
-- Gretheer 14472
-- Lapress 14473
-- Zora 14474
-- Rex Ashil 14475
-- Krellack 14476
-- Grubthor 14477
-- Huricanian 14478
-- Twilight Lord Everun 14479
-- Solenor the Slayer 14530
-- Nelson the Nice 14536
-- Creeping Doom 14761
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='14761'; -- Creeping Doom
-- Emissary Roman'khan 14862
UPDATE `creature_template` SET `HealthMultiplier`='100', `PowerMultiplier`='165' WHERE `entry`='14862'; -- Emissary Roman'khan
-- Ralo'shan the Eternal Watcher 15169
-- Rutgar Glyphshaper 15170 - npcs_rutgar_and_frankal
-- Frankal Stonebridge 15171 - npcs_rutgar_and_frankal
-- Glibb 15172
-- Calandrath 15174 - npc_innkeeper
-- Khur Hornstriker 15175
-- Vargus 15176
-- Cloud Skydancer 15177
UPDATE `creature_template` SET `DamageMultiplier`='3' WHERE `entry`='15177'; -- Cloud Skydancer
-- Runk Windtamer 15178
UPDATE `creature_template` SET `DamageMultiplier`='3' WHERE `entry`='15178'; -- Runk Windtamer
-- Mishta 15179
-- Baristolth of the Shifting Sands 15180
-- Commander Mar'alith 15181
-- Vish Kozus 15182
-- Geologist Larksbane 15183
-- Cenarion Hold Infantry 15184 - guard_contested
UPDATE `creature_template` SET `HealthMultiplier`='8' WHERE `entry`='15184'; -- Cenarion Hold Infantry
-- Brood of Nozdormu 15185
-- Beetix Ficklespragg 15189
-- Noggle Ficklespragg 15190
-- Windcaller Proudhorn 15191
-- Hermit Ortell 15194
UPDATE `creature_template` SET `MaxLevel`='58' WHERE `entry`='15194'; -- Hermit Ortell
-- Deathclasp 15196
-- Twilight Keeper Mayna 15200
-- Twilight Flamereaver 15201
-- Vyral the Vile 15202
-- Prince Skaldrenox 15203
UPDATE `creature_template` SET `HealthMultiplier`='125', `PowerMultiplier`='10' WHERE `entry`='15203'; -- Prince Skaldrenox
-- High Marshal Whirlaxis 15204
UPDATE `creature_template` SET `HealthMultiplier`='125', `PowerMultiplier`='10' WHERE `entry`='15204'; -- High Marshal Whirlaxis
-- Baron Kazum 15205
UPDATE `creature_template` SET `HealthMultiplier`='125', `PowerMultiplier`='10', `ArmorMultiplier`='1.25' WHERE `entry`='15205'; -- Baron Kazum
-- The Duke of Cynders 15206
UPDATE `creature_template` SET `HealthMultiplier`='20' WHERE `entry`='15206'; -- The Duke of Cynders
-- The Duke of Fathoms 15207
UPDATE `creature_template` SET `HealthMultiplier`='20', `PowerMultiplier`='5' WHERE `entry`='15207'; -- The Duke of Fathoms
-- The Duke of Shards 15208
UPDATE `creature_template` SET `HealthMultiplier`='20', `ArmorMultiplier`='1.25' WHERE `entry`='15208'; -- The Duke of Shards
-- Crimson Templar 15209
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='2.7' WHERE `entry`='15209'; -- Crimson Templar
-- Azure Templar 15211
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='2.7' WHERE `entry`='15211'; -- Azure Templar
-- Hoary Templar 15212
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='2.7' WHERE `entry`='15212'; -- Hoary Templar
-- Twilight Overlord 15213
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='15213'; -- Twilight Overlord
-- Mistress Natalia Mar'alith 15215
UPDATE `creature_template` SET `Rank`='2', `HealthMultiplier`='10' WHERE `entry`='15215'; -- Mistress Natalia Mar'alith
-- The Duke of Zephyrs 15220
UPDATE `creature_template` SET `HealthMultiplier`='20', `PowerMultiplier`='5' WHERE `entry`='15220'; -- The Duke of Zephyrs
-- Huum Wildmane 15270
-- Aurel Goldleaf 15282
UPDATE `creature_template` SET `PowerMultiplier`='3' WHERE `entry`='15282'; -- Aurel Goldleaf
-- Xil'xix 15286
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='45', `PowerMultiplier`='25' WHERE `entry`='15286'; -- Xil'xix
-- Aluntir 15288
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='30', `PowerMultiplier`='25' WHERE `entry`='15288'; -- Aluntir
-- Arakis 15290
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='30', `PowerMultiplier`='25' WHERE `entry`='15290'; -- Arakis
-- Aendel Windspear 15293
-- Lord Skwol 15305
UPDATE `creature_template` SET `HealthMultiplier`='125', `PowerMultiplier`='10' WHERE `entry`='15305'; -- Lord Skwol
-- Bor Wildmane 15306
-- Earthen Templar 15307
UPDATE `creature_template` SET `Rank`='1', `HealthMultiplier`='2.7' WHERE `entry`='15307'; -- Earthen Templar
-- Twilight Prophet 15308
-- Merithra of the Dream 15378
-- Caelestrasz 15379
-- Arygos 15380
-- Anachronos the Ancient 15381 - npc_anachronos_the_ancient
-- Fandral Staghelm 15382
-- Qiraji Wasp 15414
-- Kania 15419
-- Qiraji Drone 15421
-- Qiraji Tank 15422
-- Kaldorei Infantry 15423
UPDATE `creature_template` SET `DamageMultiplier`='1', `PowerMultiplier`='2' WHERE `entry`='15423'; -- Kaldorei Infantry
-- Anubisath Conqueror 15424
-- Captain Blackanvil 15440
-- Ironforge Brigade Rifleman 15441
-- Ironforge Brigade Footman 15442
-- Janela Stouthammer 15443
-- Arcanist Nozzlespring 15444
-- Hive'Zora Abomination 15449
UPDATE `creature_template` SET `HealthMultiplier`='125' WHERE `entry`='15449'; -- Hive'Zora Abomination
-- Windcaller Yessendra 15498
-- Warden Haro 15499
-- Keyl Swiftclaw 15500
-- Windcaller Kaldon 15540
-- Twilight Marauder Morna 15541
UPDATE `creature_template` SET `PowerMultiplier`='2' WHERE `entry`='15541'; -- Twilight Marauder Morna
-- Twilight Marauder 15542
UPDATE `creature_template` SET `ArmorMultiplier`='1' WHERE `entry`='15542'; -- Twilight Marauder
-- Cenarion Outrider 15545
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='15545'; -- Cenarion Outrider
-- Elder Primestone 15570
-- Elder Bladesing 15599
-- Cenarion Scout Landion 15609
-- Cenarion Scout Azenel 15610
-- Cenarion Scout Jalia 15611
-- Krug Skullsplit 15612
-- Merok Longstride 15613
-- J.D. Shadesong 15614
-- Shadow Priestess Shai 15615
-- Orgrimmar Legion Grunt 15616
-- Orgrimmar Legion Axe Thrower 15617
-- Hive'Regal Hunter-Killer 15620
UPDATE `creature_template` SET `HealthMultiplier`='150' WHERE `entry`='15620'; -- Hive'Regal Hunter-Killer
-- Priestess of the Moon 15634
-- Jonathan the Revelator 15693
-- Squire Leoren Mal'derath 15722
-- Colossus of Zora 15740
-- Colossus of Regal 15741
-- Colossus of Ashi 15742
-- Colossal Anubisath Warbringer 15743
UPDATE `creature_template` SET `MinLevel`='63', `MaxLevel`='63' WHERE `entry`='15743'; -- Colossal Anubisath Warbringer
-- Imperial Qiraji Destroyer 15744
UPDATE `creature_template` SET `MaxLevel`='63', `PowerMultiplier`='20' WHERE `entry`='15744'; -- Imperial Qiraji Destroyer
-- Greater Anubisath Warbringer 15754
-- Greater Silithid Flayer 15756
-- Colossus Researcher Sophia 15797
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='15797'; -- Colossus Researcher Sophia
-- Colossus Researcher Nestor 15798
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='15798'; -- Colossus Researcher Nestor
-- Colossus Researcher Eazel 15799
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='15799'; -- Colossus Researcher Eazel
-- Lieutenant General Nokhor 15818
-- Orgrimmar Elite Infantryman 15853
UPDATE `creature_template` SET `MinLevel`='60' WHERE `entry`='15853'; -- Orgrimmar Elite Infantryman
-- Orgrimmar Elite Cavalryman 15854
-- Tauren Rifleman 15855
-- Tauren Primalist 15856
-- Stormwind Cavalryman 15857
-- Stormwind Infantryman 15858
UPDATE `creature_template` SET `MinLevel`='60' WHERE `entry`='15858'; -- Stormwind Infantryman
-- Stormwind Archmage 15859
-- Kaldorei Marksman 15860
-- Ironforge Infantryman 15861
-- Ironforge Cavalryman 15862
-- High Commander Lynore Windstryke 15866
-- Highlord Leoric Von Zeldig 15868
-- Malagav the Tactician 15869
-- Duke August Foehammer 15870
UPDATE `creature_template` SET `PowerMultiplier`='50' WHERE `entry`='15870'; -- Duke August Foehammer
-- Sergeant Carnes 15903
-- Dirk Thunderwood 16091
-- Cenarion Hold Reservist 16139
-- Garon Hutchins 16543
-- Chief Expeditionary Requisitioner Enkles 17068
-- Apothecary Quinard 17070
-- General Kirika 17079
-- Marshal Bluewall 17080
-- Scout Bloodfist 17081
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='17081'; -- Scout Bloodfist
-- Rifleman Torrig 17082
UPDATE `creature_template` SET `DamageMultiplier`='1' WHERE `entry`='17082'; -- Rifleman Torrig
-- Alliance Silithyst Sentinel 17765
-- Horde Silithyst Sentinel 17766

-- Pyrewood / Moonrage NPCs
UPDATE `creature_template` SET `Rank` = 1 WHERE `entry` IN (
1891, -- Pyrewood Watcher		0	3	1,7
1892, -- Moonrage Watcher		0	3	2,6
1893, -- Moonrage Sentry		0	3	2,2
1894, -- Pyrewood Sentry		0	3	2
1895, -- Pyrewood Elder			0	3	1,7
1896, -- Moonrage Elder			0	3	2,2
3528, -- Pyrewood Armorer		0	3	1,7
3529, -- Moonrage Armorer		0	3	2,3
3530, -- Pyrewood Tailor		0	3	1,7
3531, -- Moonrage Tailor		0	3	2,3
3532, -- Pyrewood Leatherworker	0	3	1,7
3533 -- Moonrage Leatherworker	0	3	2,2
);

-- -------------------------------------------------
-- Creature Substitutions
-- Patch 2.3 - Dark Iron Sentry 8504 was substituted by Dark Iron Lookout 8566 - https://www.wowhead.com/npc=8504/dark-iron-sentry#comments:id=202105
UPDATE `creature` SET `id` = 8504 WHERE `guid` IN (5846,6830,6831,6832) AND `id` = 8566;

-- https://www.wowhead.com/npc=24818/anvilrage-taskmaster#comments:id=188598 , https://www.wowhead.com/npc=24819/anvilrage-enforcer#comments:id=188597
UPDATE `creature` SET `id` = 8889 WHERE `id` = 24818; -- Anvilrage Taskmaster -> Anvilrage Overseer 8889
UPDATE `creature` SET `id` = 8890 WHERE `id` = 24819; -- Anvilrage Enforcer -> Anvilrage Warden 8890

-- -----------------------------------------------------
-- Fiora Longears c.4456 was in Theramore prior to patch 2.3 instead of Auberdine
-- Use classic-db coordinates and update quest based on this commit: https://github.com/cmangos/classic-db/commit/46a822db09cbfd5728aa32a01b77a0450a84adfb
-- http://wowwiki.wikia.com/wiki/Fiora_Longears
-- -----------------------------------------------------
UPDATE creature SET position_x=-3613.43, position_y=-4463.9, position_z=13.6227, orientation=2.97522 WHERE id=4456;
UPDATE `quest_template` SET `Details` = 'Oh, to be at sea once again!  To feel the kiss of the wind, and to have the waves rock me like my blessed mother, long ago!$B$BOh, I wish I had your fortune, good $c, for I see the sea in your future!$B$BIt\'s my job to tell eager souls of the land of Kalimdor, the land of opportunity!  If you\'re willing to try your luck across the sea, then take a ship from here to the lovely port of Theramore.  Speak there with my partner, the elf, Fiora Longears.$B$BShe\'ll start you on your Kalimdor adventure!', `Objectives` = 'Speak with Fiora Longears on the docks at Theramore in Dustwallow Marsh.', Objectives='Speak with Fiora Longears on the docks at Theramore in Dustwallow Marsh.' WHERE `entry` = 1132;
-- Also readd the questrelation for Highperch Venom q.1135 which was made unavailable after Fiora was moved
DELETE FROM creature_questrelation WHERE quest=1135;
INSERT INTO creature_questrelation (id, quest) VALUES
(4456,1135);

-- Add Spinesever 34622 to Strong Junkbox Loot 29569 - Patch 2.3
DELETE FROM `item_loot_template` WHERE `entry` = 29569 AND `item` = 34622;

-- ------------------------------------------------------------------------------
-- Ogri'la
-- ------------------------------------------------------------------------------
UPDATE `creature_template` SET `Leash`=0 WHERE `entry` IN (22281,23353,23355,23354); -- Galvanoth,Braxxus,Zarcsin,Mo'arg Incinerator

-- Patch 2.3.0 (13-Nov-2007): Swamp Gas in Zangarmarsh now produce [Motes of Water] instead of [Motes of Life].
UPDATE `skinning_loot_template` SET `item`=22575, `comments`='Mote of Life' WHERE `entry`=80005;

-- Windy Cloud likely did not exist at all until Patch 2.3
UPDATE `creature` SET `spawnMask`=0 WHERE `id`=24222;

-- -----------
-- Patch 2.3.3
-- -----------
-- Felspine the Greater 21897 - https://www.wowhead.com/npc=21897/felspine-the-greater#comments:id=222741
UPDATE `creature` SET `position_x` = -3489.003, `position_y` = 1126.262, `position_z` = 10.84562, `orientation` = 4.747295 WHERE `id` = 21897;

-- =====================
-- Misc Raid Adjustments
-- =====================

-- Patch 2.3 - Coldmist Widows no longer wipe threat when casting Poison Bolt volley.
DELETE FROM creature_ai_scripts WHERE id=1617102;
INSERT INTO creature_ai_scripts (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1617102','16171','9','5','100','1025','0','30','5000','9000','11','29293','0','0','14','-100','0','0','0','0','0','0','Coldmist Widow - Cast Poison Volley and Reset Threat (Phase 1)');

-- Patch 2.2 "Greyheart Tidecallers' Virulent poison is now less deadly."
UPDATE spell_template SET EffectBasePoints1=5999 WHERE id=39029; -- 3999

-- Patch 2.2 "Underbog Colossus' Acid Spray is now somewhat less dangerous."
UPDATE spell_template SET EffectBasePoints1=2500 WHERE id=38973; -- 1424

-- Patch 2.2 "Serpentshrine Sporebats now charge less frequently."
UPDATE creature_ai_scripts SET event_param3=7000, event_param4=9000 WHERE id=2124601; -- 13600 17600

-- Patch 2.2 "Greyheart Nether-Mages now blink less frequently."
UPDATE creature_ai_scripts SET event_param3=8000, event_param4=12000 WHERE id=2123015; -- 17000 24000

-- Patch 2.2 "Coilfang Priestesses no longer uses Holy Nova or Spirit of Redemption upon death"
UPDATE `spell_template` SET `DurationIndex` = 8 WHERE `Id` = 38587; -- compensate for aggro delay due to unitflags
DELETE FROM creature_ai_scripts WHERE id=2122006;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2122006', '21220', '6', '0', '100', '0', '0', '0', '0', '0', '11', '38589', '15', '7', '11', '38587', '15', '7', '0', '0', '0', '0', 'Coilfang Priestess - Cast Holy Nova and Cast Summon Spirit of Redemption on Death');

-- Patch 2.2 AI and data for removed Priestess Spirit
UPDATE `creature_template` SET MinLevel=71, MaxLevel=71, UnitClass=8, UnitFlags=2, MinLevelHealth=2039, MaxLevelHealth=2039, MinLevelMana=7332, MaxLevelMana=7332, AIName='EventAI' WHERE entry=22210; -- UnitFlags=256
DELETE FROM creature_ai_scripts WHERE creature_id=22210;
INSERT INTO creature_ai_scripts (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2221001', '22210', '11', '0', '100', '0', '0', '0', '0', '0', '21', '0', '0', '0', '20', '0', '0', '0', '0', '0', '0', '0', 'Priestess Spirit - Stop Combat Movement and Stop Auto Attack on Spawn'),
('2221002', '22210', '14', '0', '100', '1025', '10000', '40', '2000', '2000', '11', '38580', '12', '0', '0', '0', '0', '0', '0', '0', '0', '0', 'Priestess Spirit - Cast Greater Heal on Friendlies');

-- Rain of Chaos - "Fire damage every 1 sec. for 9 sec." - http://wowwiki.wikia.com/wiki/Illidari_Fearbringer
UPDATE `spell_template` SET `EffectAmplitude2` = 1500 WHERE `Id` = 40946; -- 1000

-- ========================
-- Misc Dungeon Adjustments
-- ========================

-- Patch 2.3 - Wandering Ghosts are now neutral to players
UPDATE `creature_template` SET Faction=16 WHERE entry IN (18556,18557,18558,18559,20310,20311,20312,20313);

-- Patch 2.3 - Nethermancer Sepethrea's Frost Attack now reduces movement speed by 25% rather than 50%.
UPDATE `spell_template` SET `EffectBasePoints2` = -51 WHERE `Id` = 45195; -- -26

-- unconfirmed Add Stun Immunity for Bonechewer Ripper 17281,18055 https://www.wowhead.com/npc=17281/bonechewer-ripper#comments:id=161178
UPDATE `creature_template` SET `MechanicImmuneMask` = `MechanicImmuneMask`|2048 WHERE `entry` IN (17281,18055);

-- ======================================
-- Custom Changes (unconfirmed or custom)
-- ======================================

-- Reduce Dropchance for Warglaive of Azzinoth to 2% each from 4% (2.4.3)
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 2 WHERE `entry` = 22917 AND `item` IN (32837,32838);

-- Spitfire Totem - health increase to 35k (custom, no evidence or patch note for this)(Use as pre 2.3)
UPDATE spell_template SET EffectBasePoints1=34999 WHERE id = 38236; -- 24999

-- More items that were added in 2.2 but we missed on launch. Don't allow crafting
UPDATE spell_template SET ReagentCount1=999999 WHERE Effect1=24 AND EffectItemType1 IN(32854,34086,33791,34482,34099);

-- Remove Salvation from Lady Jaina Proudmoore & Thrall in Hyjal
UPDATE `creature_template_addon` SET `auras` = NULL WHERE `entry` IN (17772,17852);

