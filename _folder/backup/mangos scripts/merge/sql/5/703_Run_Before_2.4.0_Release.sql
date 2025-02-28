-- This file should only be run BEFORE 2.4.0 is released
-- ==================================================================================================
-- ==========         Patch Release 2.4.0 Content Reversions - Fury of the Sunwell         ==========
-- ==================================================================================================

-- --------------------------------
-- Attunements removed in Patch 2.4
-- --------------------------------
-- Karazhan
-- UPDATE areatrigger_teleport SET required_item=24490, required_quest_done=9838 WHERE id IN (4131,4135); 
-- Mount Hyjal
-- UPDATE areatrigger_teleport SET required_quest_done=10445 WHERE id IN (4319,4313,4312,4311);
-- Black Temple
-- UPDATE areatrigger_teleport SET required_quest_done=10985, status_failed_text='You must be level 70+, in a raid group and possess The Medallion of Karabor.' WHERE id IN (4598);

-- ---------------------------------
-- MGT and SWP released in Patch 2.4
-- ---------------------------------

-- Moonwell
alter table creature add column modelid int after spawnMask;
alter table creature add column equipment_id int after modelid;
alter table creature add column currentwaypoint int after spawndist;
alter table creature add column curhealth int after currentwaypoint;
alter table creature add column curmana int after curhealth;
alter table creature add column DeathState int after curmana;


-- Magister's Terrace
UPDATE areatrigger_teleport SET required_level=80 WHERE id IN (4887);
UPDATE creature SET spawnMask=1 WHERE guid=180003; -- Spawn Dungeon Messenger (Magister's Terrace)
UPDATE game_event SET EventGroup=0 WHERE entry IN(1007,1023); -- remove dailies
-- Sunwell Plateau
UPDATE areatrigger_teleport SET required_level=80 WHERE id IN (4889);
UPDATE creature SET spawnMask=1 WHERE guid=180004; -- Spawn Raid Messenger (Sunwell Plateau)

-- --------------------------------------------------------------------------------------------------------------------------------------
-- Patch 2.4 - You may now fight Prince Kael'thas and Lady Vashj without first killing all the other bosses in their respective dungeons.
-- --------------------------------------------------------------------------------------------------------------------------------------
-- UPDATE gameobject SET state=1 WHERE id IN (184326,184327,184328,184329); -- Close all doors leading to Kael in The Eye
-- UPDATE gameobject_template SET flags=flags|16 WHERE entry=184568; -- Lady Vashj Bridge Console - add GO_FLAG_NO_INTERACT

-- -------------------------------------------------------------------------------------- 
-- If the player somehow manages to get to Quel'Danas before its release, teleport him out
-- -------------------------------------------------------------------------------------- 
DELETE FROM spell_area WHERE spell=42202 AND area=4080;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_start_active`, `quest_end`, `condition_id`, `aura_spell`, `racemask`, `gender`, `autocast`) VALUES 
(42202, 4080, 0, 0, 0, 0, 0, 0, 2, 1);

-- -------------------------------------------------------------------------------------- 
-- Don't allow players to gain any rep from any source with Shattered Sun Offensive
-- -------------------------------------------------------------------------------------- 
DELETE FROM reputation_reward_rate WHERE faction=1077;
INSERT INTO reputation_reward_rate (faction, quest_rate, creature_rate, spell_rate) VALUES
(1077,0,0,0);

-- Patch 2.4 - Scale of the Sands reputation will now be awarded in Hyjal at a much higher rate.
-- https://wowwiki.fandom.com/wiki/Scale_of_the_Sands?direction=next&oldid=823544
-- 3	Mobs
-- 15	Frost Wyrm
-- 200	Bosses
-- 500	Archimonde
/*
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 200	WHERE `creature_id` = 17808; -- Anetheron			(375)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 500	WHERE `creature_id` = 17968; -- Archimonde			(1500)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 200	WHERE `creature_id` = 17842; -- Azgalor				(375)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 200	WHERE `creature_id` = 17888; -- Kaz'rogal			(375)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 200	WHERE `creature_id` = 17767; -- Rage Winterchill	(375)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17899; -- Shadowy Necromancer	(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 15	WHERE `creature_id` = 17907; -- Frost Wyrm			(60)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17898; -- Abomination			(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17895; -- Ghoul				(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17905; -- Banshee				(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17897; -- Crypt Fiend			(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17906; -- Gargoyle			(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17908; -- Giant Infernal		(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17916; -- Fel Stalker			(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17902; -- Skeleton Invader	(12)
UPDATE `creature_onkill_reputation` SET `RewOnKillRepValue1` = 3	WHERE `creature_id` = 17903; -- Skeleton Mage		(12)
*/

-- -------------------------------------
-- Cro Threadstrong didn't sell apples until patch 2.4
-- -------------------------------------
DELETE FROM `npc_vendor` WHERE `entry`=19196 AND `item`=38518;

-- =================================================================
-- Pre 2.4.0 Badge of Justice Only Dropped From Karazhan and Zul'Aman Raid Bosses
-- =================================================================
-- DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (21216,21217,21215,21214,21213,21212); -- Serpentshrine Cavern
-- DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (19516,19514,18805,19622); -- Tempest Keep: The Eye
DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (17767,17808,17888,17842,17968); -- Hyjal Summit
DELETE FROM creature_loot_template WHERE item=29434 AND entry IN (22887,22898,22841,22871,22948,22947,22949,22950,22951,22952,22917,23420); -- Black Temple

-- ----------------------------------------------------------------------------------------------
-- Patch 2.4.0 - All 25-player raid bosses that drop set tokens will now drop an additional token!
-- ----------------------------------------------------------------------------------------------
-- Do not reduce count if its patch 2.0.3 and and altered in file 702 to maxcount = 1
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 17257 AND item = 40400 AND maxcount = 3; -- Magtheridon
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 18831 AND item = 40300 AND maxcount = 3; -- High King Maulgar
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 19044 AND item = 40302 AND maxcount = 3; -- Gruul
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 19516 AND item = 36004 AND maxcount = 3; -- Void Reaver
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 19622 AND item = 36010 AND maxcount = 3; -- Kael'thas Sunstrider
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 21215 AND item = 36021 AND maxcount = 3; -- Leotheras
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 21214 AND item = 36023 AND maxcount = 3; -- Fathom-Lord Karathress
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 21212 AND item = 36026 AND maxcount = 3; -- Lady Vashj
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 17842 AND item = 36108 AND maxcount = 3; -- Azgalor
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 17968 AND item = 36107 AND maxcount = 3; -- Archimonde
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 22947 AND item = 36128 AND maxcount = 3; -- Mother Shahraz
DELETE FROM creature_loot_template WHERE entry = 22951 AND item IN (12005,36130); -- Illidari Council - Lady Malande
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
(22951, 12005, 100, 1, -12005, 1, 0, 'Epic Gem - TBC'); -- Has an Epic Gem befor getting a token Drop, which is removed when she gets the token
UPDATE creature_loot_template SET maxcount = 2 WHERE entry = 22917 AND item = 36132 AND maxcount = 3; -- Illidan Stormrage

-- Ancient Gem Vein 185557
-- Mining nodes in MH don't appear to drop green gems anymore, have a higher rate on epic gems and now drop a grey fragment (i.34907 1-3) that sells for 30g/stack (20 stack).
-- According to worldofraids.com as of patch 2.4 -> mmo-champion.com
DELETE FROM `gameobject_loot_template` WHERE `entry` = 22046;
INSERT INTO `gameobject_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
(22046, 1, 50, 2, -12004, 1, 0, 'Uncommon Gem - TBC'), -- Removed in 2.4
(22046, 12004, 20, 1, -12004, 1, 0, 'Rare Gem - TBC'),
(22046, 12005, 33, 1, -12005, 1, 0, 'Epic Gem - TBC');

-- --------------------------------------------------------------------------------------------------------------------------------------------
-- Prior to 2.4 M'uru Was located beneath the Blood Knight Headquarters in Silvermoon's Farstriders' Square (In 2.4 Moved into Sunwell Plateau)
-- --------------------------------------------------------------------------------------------------------------------------------------------
DELETE FROM creature WHERE id=17544 AND guid=105000;
INSERT INTO creature (`guid`, `id`, `map`, `spawnMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecsmin`, `spawntimesecsmax`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `DeathState`, `MovementType`) values
('105000','17544','530','1','0','0','9850.892578','-7522.771484','-9.15661','1.58193','604800','604800','0','0','7588','0','0','0');

-- existing 5 Blood Elf Magisters in the room become the channelers
DELETE FROM creature_addon WHERE guid IN (96978,96979,96980,96981,96982);
UPDATE creature SET position_x=9866.21, position_y=-7522.65, position_z=-0.489831, orientation=3.14878, MovementType=2 WHERE guid=96978;
UPDATE creature SET position_x=9850.82, position_y=-7537.75, position_z=-0.488966, orientation=1.67616, MovementType=2 WHERE guid=96979;
UPDATE creature SET position_x=9836.02, position_y=-7522.39, position_z=-0.487175, orientation=0.0307555, MovementType=2 WHERE guid=96980;
UPDATE creature SET position_x=9859.74, position_y=-7504.79, position_z=-4.00185, orientation=4.29154, MovementType=2 WHERE guid=96981;
UPDATE creature SET position_x=9842.22, position_y=-7504.77, position_z=-4.00517, orientation=5.1869, MovementType=2 WHERE guid=96982;

DELETE FROM creature_movement WHERE id BETWEEN 96978 AND 96982;
INSERT INTO creature_movement (id, point, PositionX, PositionY, PositionZ, orientation, waittime, ScriptId, comment) VALUES
(96978, 1, 9866.21, -7522.65, -0.489831, 3.14878, 100000, 1784503, ''),
(96979, 1, 9850.82, -7537.75, -0.488966, 1.67616, 100000, 1784503, ''),
(96980, 1, 9836.02, -7522.39, -0.487175, 0.0307555, 100000, 1784503, ''),
(96981, 1, 9859.74, -7504.79, -4.00185, 4.29154, 100000, 1784503, ''),
(96982, 1, 9842.22, -7504.77, -4.00517, 5.1869, 100000, 1784503, '');

DELETE FROM dbscripts_on_creature_movement WHERE id IN (1784503);
INSERT INTO dbscripts_on_creature_movement (id, delay, command, datalong, datalong2, datalong3, buddy_entry, search_radius, data_flags, dataint, dataint2, dataint3, dataint4, x, y, z, o, comments) VALUES
(1784503, 0, 15, 31324, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Magister - Cast Drain Naaru');

-- Drain Naaru should target "Ye Olde Channel Imp", not self (was 17845)
UPDATE spell_script_target SET targetEntry=17847 WHERE entry=31324;

-- Lady Liadrin
-- Patch 2.4.0 (2008-03-25): Relocated from Silvermoon City to Shattrath City. (where she has special event with A'dal)
DELETE FROM creature WHERE guid=1007112;
INSERT INTO creature (guid, id, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecsmin, spawntimesecsmax, spawndist, currentwaypoint, curhealth, curmana, DeathState, MovementType) VALUES
('1007112', '17076', '530', '1', '0', '0', '9862.18', '-7518.31', '-8.06462', '2.1435', '360', '360', '0', '0', '0', '0', '0', '0');
UPDATE creature_template SET ExtraFlags=ExtraFlags&~1 WHERE entry=17076; -- remove CREATURE_FLAG_EXTRA_INSTANCE_BIND - she has this because she's also in SWP event?

-- Magister Astalor Bloodsworn
-- this gossip text is for post 2.4 only - unknown if he had any text prior to 2.4
UPDATE creature_template SET GossipMenuId=0, NpcFlags=2 WHERE entry=17718; -- 9142

-- Patch 2.4 - The following creatures have had their hit points and damage significantly reduced: Collidus the Warp-Watcher, Fulgorge, Hemathion, Kraator, Marticar, Morcrush, and Nuramoc
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 80115, `MaxLevelHealth` = 80115, `MinMeleeDmg` = 1379, `MaxMeleeDmg` = 1918 WHERE `entry` = 18678; -- Fulgorge
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 82905, `MaxLevelHealth` = 82905, `MinMeleeDmg` = 724, `MaxMeleeDmg` = 1010 WHERE `entry` = 18680; -- Marticar
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 98130, `MaxLevelHealth` = 98130, `MinMeleeDmg` = 1708, `MaxMeleeDmg` = 2406 WHERE `entry` = 18690; -- Morcrush
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 98130, `MaxLevelHealth` = 98130, `MinMeleeDmg` = 1846, `MaxMeleeDmg` = 2602 WHERE `entry` = 18692; -- Hemathion
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 98130, `MaxLevelHealth` = 98130, `MinMeleeDmg` = 1846, `MaxMeleeDmg` = 2602 WHERE `entry` = 18694; -- Collidus the Warp-Watcher
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 98130, `MaxLevelHealth` = 98130, `MinMeleeDmg` = 1846, `MaxMeleeDmg` = 2602 WHERE `entry` = 18696; -- Kraator
UPDATE creature_template SET `rank` = 2, `HealthMultiplier` = 15, `DamageMultiplier` = 8, `MinLevelHealth` = 104790, `MaxLevelHealth` = 104790, `MinMeleeDmg` = 2020, `MaxMeleeDmg` = 2856 WHERE `entry` = 20932; -- Nuramoc

-- Npcs that were elite befor Patch 2.4 - some were forgotten in the big 2.3 nerf
-- Values taken from classic-db
-- elite at least until 2.3.2
UPDATE creature_template SET `Rank` = 1, `HealthMultiplier` = 3, `DamageMultiplier` = 2.5, `MinLevelHealth` = 5148, `MaxLevelHealth` = 1848 WHERE `entry` = 2726; -- Scorched Guardian
UPDATE creature_template SET `Rank` = 1, `HealthMultiplier` = 5, `DamageMultiplier` = 3.7, `MinLevelHealth` = 11075, `MaxLevelHealth` = 11075 WHERE `entry` = 2757; -- Blacklash
UPDATE creature_template SET `Rank` = 1, `HealthMultiplier` = 6, `DamageMultiplier` = 3.7, `MinLevelHealth` = 13290, `MaxLevelHealth` = 13290 WHERE `entry` = 2759; -- Hematus

-- --------------------------------------
-- Deathwing Brood Cloak is BoP Until 2.4
-- --------------------------------------
UPDATE item_template SET bonding=1 WHERE entry IN (31942);

-- ------------------------------------------------
-- Primal Nether and Nether Vortex is BoP Until 2.4
-- ------------------------------------------------
UPDATE item_template SET bonding=1 WHERE entry IN (23572,30183);

-- ------------------------------------------------------------------------------------------------
-- Shattrath Flask of Pure Death and Shattrath Flask of Blinding Light Were not Available Until 2.4
-- ------------------------------------------------------------------------------------------------
DELETE FROM npc_vendor_template WHERE item IN (35716,35717);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (35716,35717);

-- ---------------------------------------
-- Guild Vault Was Added To Ratchet in 2.4
-- ---------------------------------------
-- UPDATE gameobject SET spawnMask=0 WHERE guid IN (14641) AND id=187299;

-- -----------------------------------------
-- Mailbox at Darnassus Inn Was Added in 2.4
-- -----------------------------------------
UPDATE gameobject SET spawnMask=0 WHERE guid IN (49832) AND id=188123;

-- ----------------------------------------------------------------------------
-- Old Man Barlo <Master of Fishing> with Daily Quests were not added until 2.4
-- ----------------------------------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (25580);

-- ===================================================================================
-- Shattrath City (Terrace of Light) - Pre 2.4.0 Changes (Major NPC Placement Changes)
-- ===================================================================================

-- ------------------------------------------
-- V'eru Was on Main Floor Before 2.4.0 Patch
-- ------------------------------------------
UPDATE creature SET position_x = -1879.053, position_y = 5385.17, position_z = -12.34479, orientation = 1.308997 WHERE id = 22497;

-- -----------------------------------------------------------
-- G'eras and Vindicators Was on Main Floor Before 2.4.0 Patch
-- -----------------------------------------------------------
UPDATE creature SET position_x = -1846.896, position_y = 5479.625, position_z = -12.34478, orientation = 4.39823 WHERE id = 18525; -- G'eras

UPDATE creature SET position_x = -1842.875, position_y = 5477.089, position_z = -12.34482, orientation = 4.206244 WHERE guid = 96659 AND id = 20331; -- G'eras Vindicator
UPDATE creature SET position_x = -1852.048, position_y = 5485.352, position_z = -12.34477, orientation = 4.485496 WHERE guid = 96660 AND id = 20331; -- G'eras Vindicator
UPDATE creature SET position_x = -1851.577, position_y = 5479.886, position_z = -12.34478, orientation = 4.39823 WHERE guid = 96661 AND id = 20331; -- G'eras Vindicator
UPDATE creature SET position_x = -1839.709, position_y = 5481.176, position_z = -12.34482, orientation = 4.171337 WHERE guid = 96662 AND id = 20331; -- G'eras Vindicator

-- ------------------------------------------------------------------------
-- Remove 2.4.0 G'eras Vendor Item (Nether Vortex Started Selling in 2.4.0)
-- ------------------------------------------------------------------------
DELETE FROM npc_vendor WHERE entry=18525 AND item IN (30183);

-- -------------------------------------------------------------
-- Remove Portal in Shattrath City to Shattered Sun Staging Area
-- -------------------------------------------------------------
UPDATE gameobject SET spawnMask=0 WHERE id IN (187056);

-- -------------------------------------------------------------
-- Change Shattered Sun NPCs into Aldor NPCs in Terrace of Light
-- -------------------------------------------------------------
UPDATE creature SET id=19161, modelid=0 WHERE id IN(25134,25135,25136,25137) AND guid IN(96611,96600,96610,96612,96595,96603,96596,96604); -- Shattered Sun Trainee -> Neophyte Combatant
UPDATE creature SET id=19153, modelid=0 WHERE id IN (25134,25135,25136,25137); -- Shattered Sun Trainee -> Aldor Neophyte

UPDATE creature SET id=19165, modelid=0 WHERE id=25138; -- Captain Dranarus -> Veteran Vindicator
UPDATE creature SET id=19193, modelid=0 WHERE id=25140; -- Lord Torvos -> High Exarch Commodus
UPDATE creature SET id=19272, modelid=0 WHERE id=25141; -- Commander Steele -> Harbinger Argomen
UPDATE creature SET id=19337, modelid=0 WHERE id=25142; -- Shattered Sun Marksman -> Aldor Marksman
UPDATE creature SET id=19165, modelid=0 WHERE id=25143; -- Shattered Sun Veteran -> Veteran Vindicator
UPDATE creature SET id=19153, modelid=0 WHERE id=25153; -- Shattered Sun Magi -> Aldor Neophyte
UPDATE creature SET id=19142, modelid=0 WHERE id=25155; -- Shattered Sun Cleric -> Aldor Anchorite

UPDATE creature_template SET Faction=1743 WHERE entry=19216; -- Grand Anchorite Almonen had a different faction pre 2.4
UPDATE creature_template SET Faction=1743 WHERE entry=19475; -- Harbinger Haronem had a different faction pre 2.4

-- Veteran Vindicator should use the path of Captain Dranarus
DELETE FROM creature_movement WHERE id=96619;
INSERT INTO `creature_movement` (`id`, `point`, `PositionX`, `PositionY`, `PositionZ`, `waittime`, `ScriptId`, `orientation`) VALUES 
(96619, 1, -1835.49, 5313.03, -12.4282, 0, 0, 4.08568),
(96619, 2, -1841.81, 5311.87, -12.4282, 0, 0, 2.81568),
(96619, 3, -1842.62, 5313.06, -12.4282, 0, 0, 1.99573),
(96619, 4, -1842.62, 5313.06, -12.4282, 30000, 2513801, 2.47837),
(96619, 5, -1845.9, 5310.6, -12.4282, 0, 0, 3.60122),
(96619, 6, -1850.46, 5310.65, -12.4282, 0, 0, 2.63361),
(96619, 7, -1852.19, 5314.11, -12.4282, 0, 0, 1.89612),
(96619, 8, -1852.19, 5314.11, -12.4282, 30000, 2513802, 0.199661),
(96619, 9, -1850.97, 5318.61, -12.4282, 0, 0, 0.405435),
(96619, 10, -1843, 5320.51, -12.4282, 0, 0, 6.21738),
(96619, 11, -1840.26, 5319.29, -12.4282, 0, 0, 5.5121),
(96619, 12, -1840.26, 5319.29, -12.4282, 30000, 2513801, 4.50295),
(96619, 13, -1835.4, 5320.18, -12.4282, 0, 0, 5.51838),
(96619, 14, -1835.03, 5317.27, -12.4282, 0, 0, 4.82174),
(96619, 15, -1835.03, 5317.27, -12.4282, 30000, 2513802, 3.1567);

-- ---------------------------------------
-- Revert Aldor models to pre 2.4
-- ---------------------------------------
UPDATE creature_template SET ModelId1=18658, ModelId2=0, ModelId3=0, ModelId4=0 WHERE entry=19216; -- Grand Anchorite Almonen
UPDATE creature SET modelid=0 WHERE id=19216; -- Grand Anchorite Almonen

-- -----------------------------
-- Remove Misc Stuff
-- -----------------------------
UPDATE gameobject SET spawnMask=0 WHERE guid IN (47204) AND id=187345;
UPDATE creature SET spawnMask=0 WHERE id IN (25967); -- Zephyr (Keepers of Time) added in 2.4 for faster travel if Revered

-- ----------------------------
-- Remove Shattered Sun Banners
-- ----------------------------
-- HERE
UPDATE gameobject SET spawnMask=0 WHERE id IN (187356,187357);

-- ---------------------------------------------
-- Delete some pre 2.4 NPCs
-- ---------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id=23268; -- Seer Jovar

-- ----------------------------------
-- Remove Sunwell Quest Related Stuff
-- ----------------------------------
UPDATE quest_template SET MinLevel=80 WHERE entry IN (11481);

-- ---------------------------------------
-- Darthris Sunstriker did not offer quests prior to 2.4 and had faction 1738 instead of 1744
-- ---------------------------------------
DELETE FROM creature_questrelation WHERE id=18594;
UPDATE creature_template SET Faction=1738 WHERE entry=18594;

-- ------------------------------------------------------------------
-- Throne of Kil'jaeden
-- Patch 2.4.0 (25-Mar-2008): Changed to daily quest area; mobs changed.
-- ------------------------------------------------------------------

-- 22323 Incandescent Fel Spark was called Greater Fel-Spark prior to 2.4
-- TODO: buff stats too? change spells used?
UPDATE creature_template SET Scale=1.25, Name='Greater Fel-Spark' WHERE entry=22323;

UPDATE creature SET spawnMask=0 WHERE id IN(24919,24918,24933,24937,24921,24959); -- Remove 2.4.0 creatures

-- Return pre-2.4 spawns
SET @CGUID = 10060000;
DELETE FROM creature WHERE id IN (22301, 22295, 22303, 22302, 22297) AND guid BETWEEN @CGUID+0 AND @CGUID+23;
INSERT INTO creature (guid, id, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecsmin, spawntimesecsmax, spawndist, currentwaypoint, curhealth, curmana, DeathState, MovementType) VALUES
-- Throne-Guard Sentinel (22301)
(@CGUID+0, 22301, 530, 1, 0, 0, 785.132, 2097.83, 271.87, 4.86844, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+1, 22301, 530, 1, 0, 0, 779.946, 2073.83, 272.398, 0.823627, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+2, 22301, 530, 1, 0, 0, 802.046, 2079.33, 273.864, 2.73609, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+3, 22301, 530, 1, 0, 0, 787.212, 2140.05, 272.62, 1.87057, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+4, 22301, 530, 1, 0, 0, 769.439, 2157.24, 273.821, 6.06459, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+5, 22301, 530, 1, 0, 0, 791.62, 2161.98, 272.293, 3.90868, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+6, 22301, 530, 1, 0, 0, 871.568, 2079.57, 272.272, 0.649299, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+7, 22301, 530, 1, 0, 0, 891.372, 2079.28, 272.487, 2.47142, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+8, 22301, 530, 1, 0, 0, 882.59, 2102.05, 272.564, 4.63676, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+9, 22301, 530, 1, 0, 0, 883.392, 2147.3, 282.281, 2.18238, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+10, 22301, 530, 1, 0, 0, 886.167, 2168.96, 282.389, 3.95895, 900, 900, 0, 0, 0, 12924, 0, 0),
(@CGUID+11, 22301, 530, 1, 0, 0, 865.714, 2169.57, 281.366, 5.41192, 900, 900, 0, 0, 0, 12924, 0, 0),
-- Deathforge Automaton (22295)
(@CGUID+12, 22295, 530, 1, 0, 0, 847.047, 2229.8, 289.45, 3.27958, 900, 900, 0, 0, 0, 0, 0, 0),
(@CGUID+13, 22295, 530, 1, 0, 0, 844.269, 2313.19, 289.433, 3.20889, 900, 900, 0, 0, 0, 0, 0, 0),
-- Throne Hound (22303)
(@CGUID+14, 22303, 530, 1, 0, 0, 797.019, 2419.37, 281.366, 5.97975, 900, 900, 5, 0, 0, 0, 0, 2),
(@CGUID+15, 22303, 530, 1, 0, 0, 795.776, 2304, 281.373, 3.04082, 900, 900, 5, 0, 0, 0, 0, 2),
(@CGUID+16, 22303, 530, 1, 0, 0, 788.486, 2254.13, 281.363, 1.89806, 900, 900, 0, 0, 0, 0, 0, 0),
(@CGUID+17, 22303, 530, 1, 0, 0, 775.393, 2305.84, 281.364, 6.20205, 900, 900, 0, 0, 0, 0, 0, 0),
-- Throne-Guard Champion (22302)
(@CGUID+18, 22302, 530, 1, 0, 0, 798.233, 2391.41, 281.371, 5.34831, 900, 900, 0, 0, 0, 13236, 0, 0),
(@CGUID+19, 22302, 530, 1, 0, 0, 741.695, 2382.59, 275.06, 2.62693, 900, 900, 0, 0, 0, 13236, 0, 0),
(@CGUID+20, 22302, 530, 1, 0, 0, 741.69, 2543.5, 277.278, 3.8443, 900, 900, 0, 0, 0, 13236, 0, 0),
(@CGUID+21, 22302, 530, 1, 0, 0, 708.449, 2433.46, 275.059, 4.61161, 900, 900, 0, 0, 0, 13236, 0, 0),
-- Throne-Guard Highlord (22297)
(@CGUID+22, 22297, 530, 1, 0, 1014, 737.091, 2570.02, 278.461, 3.36831, 900, 900, 5, 0, 0, 13236, 0, 2),
(@CGUID+23, 22297, 530, 1, 0, 1014, 851.383, 2169.09, 281.386, 2.33316, 900, 900, 5, 0, 0, 13236, 0, 2);

DELETE FROM creature_movement WHERE id IN (@CGUID+14,@CGUID+15,@CGUID+22,@CGUID+23);
INSERT INTO creature_movement (id, point, PositionX, PositionY, PositionZ, orientation, waittime, ScriptId) VALUES
-- Throne Hound 1
(@CGUID+14, 1, 797.019, 2419.37, 281.366, 5.97975, 0, 0),
(@CGUID+14, 2, 814.088, 2413.51, 281.375, 5.83446, 0, 0),
(@CGUID+14, 3, 827.876, 2407.14, 281.371, 5.85016, 0, 0),
(@CGUID+14, 4, 842.204, 2400.51, 281.595, 5.85016, 0, 0),
(@CGUID+14, 5, 850.954, 2392.18, 281.371, 5.34123, 0, 0),
(@CGUID+14, 6, 854.9, 2385.94, 280.663, 5.14566, 0, 0),
(@CGUID+14, 7, 856.71, 2380.41, 279.316, 4.91004, 0, 0),
(@CGUID+14, 8, 855.671, 2374.94, 281.37, 4.51577, 0, 0),
(@CGUID+14, 9, 850.076, 2368.97, 281.37, 3.93693, 0, 0),
(@CGUID+14, 10, 844.039, 2363.53, 281.409, 3.85918, 0, 0),
(@CGUID+14, 11, 842.272, 2361.98, 281.465, 3.85918, 0, 0),
(@CGUID+14, 12, 839.635, 2359.68, 279.714, 3.85918, 0, 0),
(@CGUID+14, 13, 835.097, 2355.72, 282.107, 3.85918, 0, 0),
(@CGUID+14, 14, 828.739, 2351.6, 281.371, 3.66361, 0, 0),
(@CGUID+14, 15, 815.602, 2344.05, 281.371, 3.66361, 0, 0),
(@CGUID+14, 16, 806.272, 2340.79, 281.371, 3.38794, 0, 0),
(@CGUID+14, 17, 796.463, 2339.83, 281.325, 3.07221, 0, 0),
(@CGUID+14, 18, 791.819, 2340.15, 280.879, 2.96854, 0, 0),
(@CGUID+14, 19, 780.243, 2344.2, 275.348, 2.77297, 0, 0),
(@CGUID+14, 20, 771.991, 2347.99, 275.259, 2.09439, 0, 0),
(@CGUID+14, 21, 769.664, 2352.02, 275.506, 1.88076, 0, 0),
(@CGUID+14, 22, 769.991, 2356.1, 273.567, 1.4213, 0, 0),
(@CGUID+14, 23, 770.862, 2361.88, 274.903, 1.4213, 0, 0),
(@CGUID+14, 24, 771.796, 2372.92, 275.059, 1.53911, 0, 0),
(@CGUID+14, 25, 767.357, 2383.26, 275.064, 1.93181, 0, 0),
(@CGUID+14, 26, 762.622, 2395.8, 275.059, 1.93181, 0, 0),
(@CGUID+14, 27, 758.499, 2405.19, 274.768, 1.89332, 0, 0),
(@CGUID+14, 28, 757.578, 2407.95, 273.947, 1.89332, 0, 0),
(@CGUID+14, 29, 757.215, 2410.86, 274.808, 1.63179, 0, 0),
(@CGUID+14, 30, 759.094, 2416.37, 275.06, 1.14563, 0, 0),
(@CGUID+14, 31, 765.32, 2424.67, 275.075, 0.752141, 0, 0),
(@CGUID+14, 32, 772.625, 2429.3, 274.944, 0.320957, 0, 0),
(@CGUID+14, 33, 775.513, 2429.03, 274.922, 6.09128, 0, 0),
(@CGUID+14, 34, 784.174, 2426.88, 280.061, 5.84073, 0, 0),
(@CGUID+14, 35, 785.544, 2424.29, 281.003, 5.84073, 0, 0),
-- Throne Hound 2
(@CGUID+15, 1, 795.776, 2304, 281.373, 3.04082, 30000, 0),
(@CGUID+15, 2, 781.247, 2269.98, 281.367, 4.24248, 30000, 0),
(@CGUID+15, 3, 797.825, 2283.67, 281.372, 0.617861, 0, 0),
(@CGUID+15, 4, 823.101, 2292.04, 281.441, 2.4282, 30000, 0),
-- Throne-Guard Highlord 1
(@CGUID+22, 1, 737.091, 2570.02, 278.461, 3.36831, 0, 0),
(@CGUID+22, 2, 716.847, 2568.76, 278.588, 3.08949, 0, 0),
(@CGUID+22, 3, 701.678, 2569.55, 278.102, 3.08949, 0, 0),
(@CGUID+22, 4, 694.235, 2566.24, 279.241, 4.18905, 0, 0),
(@CGUID+22, 5, 692.987, 2554.09, 280.103, 4.61003, 0, 0),
(@CGUID+22, 6, 694.124, 2543.61, 277.886, 4.82051, 0, 0),
(@CGUID+22, 7, 696.673, 2528.64, 276.776, 4.85586, 0, 0),
(@CGUID+22, 8, 698.595, 2515.34, 275.752, 4.85586, 0, 0),
(@CGUID+22, 9, 700.982, 2509.09, 275.164, 4.99723, 0, 0),
(@CGUID+22, 10, 703.106, 2501.84, 277.225, 4.99723, 0, 0),
(@CGUID+22, 11, 705.706, 2491.33, 279.151, 4.75768, 0, 0),
(@CGUID+22, 12, 706.097, 2482.69, 279.423, 4.75768, 0, 0),
(@CGUID+22, 13, 706.542, 2472.89, 275.036, 3.91887, 0, 0),
(@CGUID+22, 14, 698.854, 2467.31, 275.49, 3.6432, 0, 0),
(@CGUID+22, 15, 690.065, 2463.52, 275.251, 3.52539, 0, 0),
(@CGUID+22, 16, 681.929, 2462.08, 275.067, 3.28977, 0, 0),
(@CGUID+22, 17, 675.494, 2460, 275.255, 3.72331, 0, 0),
(@CGUID+22, 18, 669.428, 2454.11, 275.746, 3.99662, 0, 0),
(@CGUID+22, 19, 667.53, 2449.09, 276.945, 4.50792, 0, 0),
(@CGUID+22, 20, 667.341, 2443.75, 276.105, 4.74354, 0, 0),
(@CGUID+22, 21, 667.542, 2438.5, 275.755, 4.9014, 0, 0),
(@CGUID+22, 22, 669.515, 2428.18, 275.666, 4.9014, 0, 0),
(@CGUID+22, 23, 670.666, 2417.18, 275.669, 4.70584, 0, 0),
(@CGUID+22, 24, 670.621, 2410.18, 275.633, 4.70584, 0, 0),
(@CGUID+22, 25, 670.706, 2402.82, 275.661, 4.74354, 0, 0),
(@CGUID+22, 26, 671.561, 2394.24, 275.598, 4.9014, 0, 0),
(@CGUID+22, 27, 673.851, 2387.06, 275.138, 5.13702, 0, 0),
(@CGUID+22, 28, 677.929, 2379.46, 275.06, 5.29489, 0, 0),
(@CGUID+22, 29, 679.903, 2376.57, 275.06, 5.49045, 0, 0),
(@CGUID+22, 30, 686.801, 2369.76, 274.947, 5.56821, 0, 0),
(@CGUID+22, 31, 689.365, 2367.54, 273.881, 5.56821, 0, 0),
(@CGUID+22, 32, 691.785, 2365.91, 274.518, 5.72372, 0, 0),
(@CGUID+22, 33, 701.53, 2360.1, 275.058, 5.72372, 0, 0),
(@CGUID+22, 34, 708.821, 2353.33, 275.058, 5.60591, 0, 0),
(@CGUID+22, 35, 718.897, 2342.27, 275.058, 5.40799, 0, 0),
(@CGUID+22, 36, 728.016, 2333.28, 275.058, 5.60591, 0, 0),
(@CGUID+22, 37, 728.016, 2333.28, 275.058, 5.60591, 0, 0),
(@CGUID+22, 38, 746.196, 2332.84, 275.075, 0.002087, 0, 0),
(@CGUID+22, 39, 759.477, 2334.54, 275.089, 0.200008, 0, 0),
(@CGUID+22, 40, 779.51, 2338.6, 275.457, 0.200008, 0, 0),
(@CGUID+22, 41, 790.967, 2340.92, 280.545, 0.200008, 0, 0),
(@CGUID+22, 42, 807.339, 2346.09, 281.372, 0.306036, 0, 0),
(@CGUID+22, 43, 824.547, 2354.44, 281.371, 0.712872, 0, 0),
(@CGUID+22, 44, 830.49, 2358.81, 281.433, 0.712872, 0, 0),
(@CGUID+22, 45, 833.672, 2361.56, 279.317, 0.712872, 0, 0),
(@CGUID+22, 46, 839.148, 2366.3, 281.332, 0.712872, 0, 0),
(@CGUID+22, 47, 844.638, 2370.44, 281.381, 0.712872, 0, 0),
(@CGUID+22, 48, 849.013, 2373.69, 281.775, 0.870737, 0, 0),
(@CGUID+22, 49, 854.929, 2381.06, 279.47, 0.950848, 0, 0),
(@CGUID+22, 50, 856.694, 2388.14, 281.371, 1.38439, 0, 0),
(@CGUID+22, 51, 853.615, 2404.09, 281.371, 1.7496, 0, 0),
(@CGUID+22, 52, 848.784, 2419.43, 281.943, 1.90511, 0, 0),
(@CGUID+22, 53, 848.146, 2424.88, 284.289, 1.36947, 0, 0),
(@CGUID+22, 54, 850.784, 2431.85, 288.954, 1.1739, 0, 0),
(@CGUID+22, 55, 852.364, 2435.62, 290.27, 1.1739, 0, 0),
(@CGUID+22, 56, 858.676, 2445.04, 290.437, 1.03646, 0, 0),
(@CGUID+22, 57, 862.602, 2452.46, 287.392, 1.19196, 0, 0),
(@CGUID+22, 58, 864.287, 2456.69, 285.313, 1.19196, 0, 0),
(@CGUID+22, 59, 865.584, 2459.95, 286.53, 1.19196, 0, 0),
(@CGUID+22, 60, 868.454, 2468.88, 288.557, 1.34983, 0, 0),
(@CGUID+22, 61, 869.221, 2472.29, 289.273, 1.34983, 0, 0),
(@CGUID+22, 62, 871.522, 2482.53, 289.642, 1.34983, 0, 0),
(@CGUID+22, 63, 873.056, 2489.36, 289.571, 1.34983, 0, 0),
(@CGUID+22, 64, 874.753, 2498.45, 292.708, 1.42759, 0, 0),
(@CGUID+22, 65, 874.852, 2506.73, 292.766, 1.58545, 0, 0),
(@CGUID+22, 66, 874.78, 2511.63, 291.382, 1.58545, 0, 0),
(@CGUID+22, 67, 874.526, 2517.34, 293.054, 1.6255, 0, 0),
(@CGUID+22, 68, 874.347, 2520.6, 292.541, 1.6255, 0, 0),
(@CGUID+22, 69, 873.187, 2526.91, 296.942, 1.78101, 0, 0),
(@CGUID+22, 70, 872.529, 2529.99, 297.649, 1.78101, 0, 0),
(@CGUID+22, 71, 869.067, 2537.99, 298.009, 2.01663, 0, 0),
(@CGUID+22, 72, 865.907, 2543.28, 298.007, 2.25225, 0, 0),
(@CGUID+22, 73, 861.598, 2545.94, 295.971, 2.48787, 0, 0),
(@CGUID+22, 74, 858.075, 2548.64, 297.853, 2.48787, 0, 0),
(@CGUID+22, 75, 847.317, 2556.88, 297.948, 2.48787, 0, 0),
(@CGUID+22, 76, 844.73, 2558.21, 297.01, 2.72349, 0, 0),
(@CGUID+22, 77, 839.161, 2559.92, 294.014, 2.88136, 0, 0),
(@CGUID+22, 78, 831.291, 2560.7, 293.689, 3.11698, 0, 0),
(@CGUID+22, 79, 820.794, 2560.96, 293.849, 3.11698, 0, 0),
(@CGUID+22, 80, 813.796, 2561.13, 293.779, 3.11698, 0, 0),
(@CGUID+22, 81, 803.305, 2561.39, 294.329, 3.11698, 0, 0),
(@CGUID+22, 82, 792.808, 2561.65, 294.46, 3.11698, 0, 0),
(@CGUID+22, 83, 781.556, 2561.93, 293.804, 3.11698, 0, 0),
(@CGUID+22, 84, 778.657, 2562.19, 293.277, 2.90492, 0, 0),
(@CGUID+22, 85, 775.229, 2562.93, 291.303, 2.92848, 0, 0),
(@CGUID+22, 86, 763.817, 2565.4, 280.965, 2.92848, 0, 0),
(@CGUID+22, 87, 756.292, 2567.03, 279.025, 2.92848, 0, 0),
-- Throne-Guard Highlord 2
(@CGUID+23, 1, 850.2, 2170.19, 281.393, 2.34572, 0, 0),
(@CGUID+23, 2, 839.964, 2180.64, 281.359, 2.34572, 0, 0),
(@CGUID+23, 3, 829.172, 2190.03, 281.278, 2.63239, 0, 0),
(@CGUID+23, 4, 820.493, 2194.88, 279.594, 2.63239, 0, 0),
(@CGUID+23, 5, 812.375, 2195.36, 276.419, 3.13191, 0, 0),
(@CGUID+23, 6, 806.01, 2192.45, 274.198, 4.27387, 0, 0),
(@CGUID+23, 7, 805.821, 2176.33, 272.477, 4.49378, 0, 0),
(@CGUID+23, 8, 804.37, 2163.01, 272.498, 4.61159, 0, 0),
(@CGUID+23, 9, 802.968, 2150.21, 272.339, 4.38776, 0, 0),
(@CGUID+23, 10, 800.015, 2141.97, 272.11, 4.10501, 0, 0),
(@CGUID+23, 11, 791.58, 2132.2, 272.242, 3.8961, 0, 0),
(@CGUID+23, 12, 787.556, 2128.84, 271.482, 3.77829, 0, 0),
(@CGUID+23, 13, 781.646, 2124.08, 272.116, 3.85604, 0, 0),
(@CGUID+23, 14, 775.472, 2116.37, 272.119, 4.09166, 0, 0),
(@CGUID+23, 15, 770.357, 2106.62, 272.204, 4.36734, 0, 0),
(@CGUID+23, 16, 768.118, 2094.03, 272.211, 4.64301, 0, 0),
(@CGUID+23, 17, 768.5, 2081.22, 272.211, 4.79852, 0, 0),
(@CGUID+23, 18, 771.37, 2066.99, 272.575, 5.03414, 0, 0),
(@CGUID+23, 19, 777.571, 2058.03, 272.609, 5.54543, 0, 0),
(@CGUID+23, 20, 787.554, 2053.45, 272.609, 5.97897, 0, 0),
(@CGUID+23, 21, 797.932, 2057.35, 272.609, 0.375945, 0, 0),
(@CGUID+23, 22, 811.58, 2064.95, 272.607, 0.375945, 0, 0),
(@CGUID+23, 23, 823.227, 2068.66, 272.574, 0.218079, 0, 0),
(@CGUID+23, 24, 834.406, 2071.89, 272.147, 0.53381, 0, 0),
(@CGUID+23, 25, 842.197, 2077.92, 272.104, 0.769429, 0, 0),
(@CGUID+23, 26, 848.32, 2085.66, 272.11, 0.964993, 0, 0),
(@CGUID+23, 27, 851.305, 2089.97, 272.063, 0.964993, 0, 0),
(@CGUID+23, 28, 853.637, 2093.34, 270.113, 0.964993, 0, 0),
(@CGUID+23, 29, 856.197, 2097.03, 272.27, 0.964993, 0, 0),
(@CGUID+23, 30, 867.69, 2107.65, 272.323, 0.838544, 0, 0),
(@CGUID+23, 31, 872.61, 2114.16, 272.431, 1.07416, 0, 0),
(@CGUID+23, 32, 877.076, 2124.78, 274.603, 1.49278, 0, 0),
(@CGUID+23, 33, 875.977, 2133.23, 281.276, 1.7661, 0, 0),
(@CGUID+23, 34, 873.42, 2136.77, 280.649, 2.23812, 0, 0),
(@CGUID+23, 35, 871.622, 2139.05, 279.004, 2.23812, 0, 0),
(@CGUID+23, 36, 867.871, 2143.81, 281.361, 2.23812, 0, 0),
(@CGUID+23, 37, 865.497, 2149.12, 281.379, 1.98994, 0, 0),
(@CGUID+23, 38, 862.876, 2155, 280.888, 1.98994, 0, 0),
(@CGUID+23, 39, 861.466, 2158.16, 279.638, 1.98994, 0, 0),
(@CGUID+23, 40, 859.315, 2162.99, 281.362, 1.98994, 0, 0);

-- ----------------------------------------------------
-- Remove 2 Halaa Daily PvP Quest Givers (Added in 2.4)
-- ----------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (24866,24881);

-- --------------------------------------------------------------------------------------------------------------
-- Remove Level 70 (Item Level 115) PvP Rare Set 2 Armor from Repuatation Vendors (Outside of Isle of Quel'Danas)
-- --------------------------------------------------------------------------------------------------------------

DELETE FROM npc_vendor WHERE item BETWEEN 35328 AND 35347;
DELETE FROM npc_vendor WHERE item BETWEEN 35356 AND 35395;
DELETE FROM npc_vendor WHERE item BETWEEN 35402 AND 35416;
DELETE FROM npc_vendor WHERE item BETWEEN 35464 AND 35478;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantID=0 WHERE entry BETWEEN 35328 AND 35347;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantID=0 WHERE entry BETWEEN 35356 AND 35395;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantID=0 WHERE entry BETWEEN 35402 AND 35416;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantID=0 WHERE entry BETWEEN 35464 AND 35478;

-- ---------------------
-- Season 4 PvP Trinkets
-- ---------------------
DELETE FROM npc_vendor WHERE item BETWEEN 37864 AND 37865;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantID=0 WHERE entry BETWEEN 37864 AND 37865;

-- ------------------------------------------------
-- Gift of Arthas (Stacks to max of 5 Prior to 2.4)
-- ------------------------------------------------
UPDATE item_template SET stackable=5 WHERE entry IN (9088); 

-- ----------------------------------------------------------
-- Recipe: Charred Bear Kabobs Was Not Introduced Until 2.4.0
-- ----------------------------------------------------------
-- DELETE FROM npc_vendor WHERE item IN (35564,35563);
-- DELETE FROM npc_vendor_template WHERE item IN (35564,35563);
-- UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (35564);
-- UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (35563);

-- --------------------------------------------------------
-- Recipe: Juicy Bear Burger Was Not Introduced Until 2.4.0
-- --------------------------------------------------------
-- DELETE FROM npc_vendor WHERE item IN (35566,35565);
-- DELETE FROM npc_vendor_template WHERE item IN (35566,35565);
-- UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (35566);
-- UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (35565);

-- -----------------------------------------------------------------------
-- Schematic: Minor Recombobulator Was Not Sold By Gagsprocket Until 2.4.0
-- -----------------------------------------------------------------------
DELETE FROM npc_vendor WHERE item IN (14639) AND entry IN (14639);

-- -------------------------------------------------------
-- Craftsman's Monocle Required Level 32 Until 2.4.0 Patch
-- -------------------------------------------------------
UPDATE item_template SET RequiredLevel=32 WHERE entry IN (4393); 

-- ------------------------------------------------------------------
-- Schematic: Rocket Boots Xtreme Lite Was Not Introduced Until 2.4.0
-- ------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE item IN (35582,35581);
DELETE FROM reference_loot_template WHERE item IN (35582,35581);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantID=0 WHERE entry IN (35582);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80, DisenchantId=0 WHERE entry IN (35581);

-- ------------------------------
-- Escape From Skettis Tuned Down
-- ------------------------------
UPDATE quest_template SET rewchoiceitemID1 = 0, rewchoiceitemID2 = 0, rewchoiceitemcount1 = 0, rewchoiceitemcount2 = 0, rewmoneymaxlevel = 85900, Rewrepfaction1 = 1031, Rewrepvalue1 = 150 WHERE entry = 11085;

-- --------------------------------------------------------------
-- Doomwalker Loot Was BoP Before 2.4.0 (Changed to BoE in 2.4.0) - Also Significantly Reduced Gold Drop (Currently Set at 50%)
-- --------------------------------------------------------------
UPDATE item_template SET bonding=1 WHERE entry IN (30722,30723,30724,30725,30726,30727,30728,30729,30730,30731);
UPDATE creature_template SET MinLootGold=97877, MaxLootGold=127819 WHERE entry IN (17711);

-- -------------------------------------------------------------------
-- Doomlord Kazzak Loot Was BoP Before 2.4.0 (Changed to BoE in 2.4.0) - Also Significantly Reduced Gold Drop (Currently Set at 50%)
-- -------------------------------------------------------------------
UPDATE item_template SET bonding=1 WHERE entry IN (30732,30733,30734,30735,30736,30737,30738,30739,30740,30741);
UPDATE creature_template SET MinLootGold=55377, MaxLootGold=72319 WHERE entry IN (18728);

-- ------------------------------------------------------------------------
-- Epic gems from Heroic difficulty dungeons were Unique-Equipped until 2.4
-- ------------------------------------------------------------------------
UPDATE item_template SET flags=flags|524288 WHERE entry IN (30546,30547,30548,30549,30550,30551,30552,30553,30554,30555,30556,30558,30560,30563,30564,30565,30566,30572,30573,30574,30575,30581,30582,30583,30584,30585,30586,30587,30588,30589,30590,30591,30592,30593,30594,30600,30601,30602,30603,30604,30605,30606,30607,30608);

-- ---------------------------------------------------------------------------
-- All weaponsmith crafted 1-handers should be main hand until 2.4
-- ---------------------------------------------------------------------------
UPDATE item_template SET InventoryType = 21, ExtraFlags=ExtraFlags|2 WHERE entry IN (28437,28438,28439); -- Drakefist Hammer, Dragonmaw, Dragonstrike
UPDATE item_template SET InventoryType = 21, ExtraFlags=ExtraFlags|2 WHERE entry IN (28431,28432,28433); -- The Planar Edge, Black Planar Edge, Wicked Edge of the Planes
UPDATE item_template SET InventoryType = 21, ExtraFlags=ExtraFlags|2 WHERE entry IN (28425,28426,28427); -- Fireguard, Blazeguard, Blazefury

-- ---------------------------------------------------------------------------------
-- Pit Lord's Satchel and Black Sack of Gems did not drop from Magtheridon until 2.4
-- ---------------------------------------------------------------------------------
DELETE FROM creature_loot_template WHERE entry = 17257 AND item IN (34845,34846);

-- ---------------
-- Skyguard Khatie
-- ---------------
-- was not mounted and different location before 2.4
UPDATE creature SET position_x=2518.76, position_y=7353.99, position_z=380.734, orientation=2.91244 WHERE id=23335;
DELETE FROM creature_template_addon WHERE entry=23335;

-- -------------------------------------------------
-- Arcane Guardian - text was changed in Patch 2.4.0
-- -------------------------------------------------
-- UPDATE creature_ai_texts SET content_default='Remain strong. Kael''thas will lead you to power and glory!' WHERE entry=-1334;

-- ------------------------------------------------------------------------------------------
-- Silvermoon City Guardian - new text added in Patch 2.4 - revert it to one of the old texts
-- ------------------------------------------------------------------------------------------
-- same as 2000000477, but better than deleting it entirely so id cannot be misconstrued as free
-- text is a response to Champion Vranesh (18146) when he moves along his route
-- UPDATE dbscript_string SET content_default='Your power strengthens us all.', broadcast_text_id=14792 WHERE entry=2000001490; -- 'Glory to the Sun K-- ah... Silvermoon!'

-- Quests Cut Arathor Supply Lines were removed from World of Warcraft in patch 2.4.0.
-- https://wowwiki.fandom.com/wiki/Quest:Cut_Arathor_Supply_Lines_(level_20)
DELETE FROM creature_questrelation WHERE quest IN (8123,8160,8161,8162,8299);
DELETE FROM creature_involvedrelation WHERE quest IN (8123,8160,8161,8162,8299);
INSERT INTO `creature_questrelation` (`id`, `quest`) VALUES
(15022, 8123),(15022, 8160),(15022, 8161),(15022, 8162),(15022, 8299);
INSERT INTO `creature_involvedrelation` (`id`, `quest`) VALUES
(15022, 8123),(15022, 8160),(15022, 8161),(15022, 8162),(15022, 8299);

-- ----------------------------------------------------------------------------
-- Patch 2.4.0 (25-Mar-2008): Undocumented change - A Study in Power quest text
-- ----------------------------------------------------------------------------
-- text changed to accommodate the events surrounding M'uru. For the old version, see:
-- http://wowwiki.wikia.com/index.php?title=Quest%3AA_Study_in_Power&diff=next&oldid=1229888
UPDATE quest_template SET Details='Do not think me unduly harsh, $N. Stillblade knew well what he was being sent to do. He understood and accepted his duty and is an example for all aspiring Blood Knights.$B$BMy words are no eulogy, $N. Stillblade''s service to us is far from over and you will be the instrument of his resurrection.$B$BYour education in the use of the Light has focused on mere charms and parlor tricks thus far. The time has come for you to learn of the nature and breadth of our power. Seek out Magister Astalor Bloodsworn.' WHERE entry=9681;
UPDATE quest_template SET OfferRewardText='Welcome, young $C.$B$BThe power of the Blood Knights is taken from a being of immense power.$B$B<The magister gestures to the large, captive being in the center of the room.>$B$BIsn''t it magnificent? ''Twas a gift from our beloved Prince Kael''thas, and it is the foundation of the Blood Knights'' mastery of the Light. This creature begrudges us its power, so we have devised a method for claiming it on our own terms.' WHERE entry=9681;

-- ----------------------------------------------------------------------------
-- gems sold by http://www.wowhead.com/npc=19538/dealer-senzik#comments:id=269159 should have a base price of 4g befor Patch 2.4.0
-- ----------------------------------------------------------------------------
UPDATE `item_template` SET `BuyPrice`=40000 WHERE `entry` IN (23095,28595,23113,23106,23097,23105,23114,23100,23108,23098,23104,23099,23121,23101,23103,23116,23109,23096,23110,28290,23118,23111,23119,23120,23094,23115);

-- --------------------------------------------------------------------------------------------------
-- Emissary Mordin c.19202 had a different faction and did not offer quests prior to 2.4.0
-- --------------------------------------------------------------------------------------------------
UPDATE creature_template SET Faction=1743 WHERE entry=19202;
DELETE FROM creature_questrelation WHERE id=19202;

-- -------------------------------------------------------------------------------------------------
-- Patch 2.4.0 - Non-corporeal Undead and Mechanical creatures are now susceptible to bleed effects.
-- Patch 2.4.0 - Elemental creatures are no longer explicitly immune to poison and disease effects.

-- We have no easy way to implement the disease immunity for elementals currently
-- -------------------------------------------------------------------------------------------------

-- Add poison immunity to all elementals
UPDATE creature_template SET `ExtraFlags`=`ExtraFlags`|16777216 WHERE CreatureType=4;

-- Add bleed immunity to all mechanicals
UPDATE creature_template SET `MechanicImmuneMask` = `MechanicImmuneMask`|16384 WHERE CreatureType=9 AND Entry NOT IN (100700,100800,100801);
-- Add bleed immunity to all non-corporeal undead
UPDATE creature_template SET `MechanicImmuneMask` = `MechanicImmuneMask`|16384 WHERE `entry` IN (
302, -- Blind Mary
392, -- Captain Grayson
1157, -- Cursed Sailor
1158, -- Cursed Marine
1531, -- Lost Soul
1532, -- Wandering Spirit
1533, -- Tormented Spirit
1534, -- Wailing Ancestor
1655, -- Nissa Agamand
1787, -- Skeletal Executioner
1789, -- Skeletal Acolyte
1798, -- Tortured Soul
1800, -- Cold Wraith
1801, -- Blood Wraith
1802, -- Hungering Wraith
1804, -- Wailing Death
1849, -- Dreadwhisper
1946, -- Lillith Nefara
1983, -- Nightlash
3863, -- Lupine Horror
8523, -- Scourge Soldier
8524, -- Cursed Mage
8525, -- Scourge Warder
8526, -- Dark Caster
8527, -- Scourge Guard
8528, -- Dread Weaver
8529, -- Scourge Champion
8542, -- Death Singer
10390, -- Skeletal Guardian
10391, -- Skeletal Berserker
10394, -- Black Guard Sentry
10436, -- Baroness Anastari
10463, -- Shrieking Banshee
10464, -- Wailing Banshee
10478, -- Splintered Skeleton
10482, -- Risen Lackey
10485, -- Risen Aberration
10486, -- Risen Warrior
10487, -- Risen Protector
10488, -- Risen Construct
10489, -- Risen Guard
10491, -- Risen Bonewarder
10498, -- Spectral Tutor
10499, -- Spectral Researcher
10500, -- Spectral Teacher
10503, -- Jandice Barov
12178, -- Tortured Druid 
12179, -- Tortured Sentinel

-- TBC+
15117, -- Chained Spirit
15547, -- Spectral Charger
15548, -- Spectral Stallion
15551, -- Spectral Stable Hand
15656, -- Angershade
15657, -- Darkwraith
15720, -- Timbermaw Ancestor
16066, -- Spectral Assassin
16093, -- Spectral Stalker
16127, -- Spectral Trainee
16143, -- Shadow of Doom
16148, -- Spectral Deathknight
16149, -- Spectral Horse
16150, -- Spectral Rider
16164, -- Shade of Naxxramas
16194, -- Unholy Axe
16215, -- Unholy Staff
16216, -- Unholy Swords
16248, -- Jurion the Deceiver
16249, -- Masophet the Black
16298, -- Spectral Soldier
16311, -- Phantasmal Watcher
16312, -- Spectral Screamer
16314, -- Fallen Ranger
16320, -- Eye of Dar'Khan
16321, -- Wailer
16323, -- Phantasmal Seeker
16325, -- Quel'dorei Ghost
16326, -- Quel'dorei Wraith
16327, -- Ravening Apparition
16328, -- Vengeful Apparition
16379, -- Spirit of the Damned
16389, -- Spectral Apprentice
16406, -- Phantom Attendant
16407, -- Spectral Servant
16408, -- Phantom Valet
16409, -- Phantom Guest
16410, -- Spectral Retainer
16411, -- Spectral Chef
16412, -- Ghostly Baker
16414, -- Ghostly Steward
16415, -- Skeletal Waiter
16423, -- Spectral Apparition
16424, -- Spectral Sentry
16425, -- Phantom Guardsman
16429, -- Soul Weaver
16437, -- Spectral Spirit
16449, -- Spirit of Naxxramas
16468, -- Spectral Patron
16470, -- Ghostly Philanthropist
16471, -- Skeletal Usher
16472, -- Phantom Stagehand
16473, -- Spectral Performer
16481, -- Ghastly Haunt
16482, -- Trapped Soul
16524, -- Shade of Aran
16525, -- Spell Shade
16526, -- Sorcerous Shade
16775, -- Spirit of Mograine
16776, -- Spirit of Blaumeux
16777, -- Spirit of Zeliek
16778, -- Spirit of Korth'azz
16904, -- Unyielding Footman
16905, -- Unyielding Sorcerer
16906, -- Unyielding Knight
16977, -- Arch Mage Xintor
16978, -- Lieutenant Commander Thalvos
17007, -- Lady Keira Berrybuck
17067, -- Phantom Hound
17086, -- Enraged Wraith
17415, -- Lordaeron Mage
17466, -- Lordaeron Spirit
17533, -- Romulo
17534, -- Julianne
17588, -- Veridian Whelp
17589, -- Veridian Broodling
17592, -- Razormaw
17612, -- Quel'dorei Magewraith
17672, -- Deadwind Villager
17714, -- Bloodcursed Voyager
17905, -- Banshee
18043, -- Agitated Orc Spirit
18254, -- Shadow of Aran
18478,20303, -- Avatar of the Martyred (1)
18441,20305, -- Stolen Soul (1)
18503,20309, -- Phantasmal Possessor
18557,20310, -- Phasing Cleric (1)
18556,20311, -- Phasing Soldier
18558,20312, -- Phasing Sorcerer
18559,20313, -- Phasing Stalker
18500,20320, -- Unliving Cleric
18498,20321, -- Unliving Soldier (1)
18499,20322, -- Unliving Sorcerer (1)
18501,20323, -- Unliving Stalker (1)
18327,20691, -- Time-Lost Controller (1)
18319,20697, -- Time-Lost Scryer (1)
18320,20698, -- Time-Lost Shadowmage (1)
18872, -- Disembodied Vindicator
18873, -- Disembodied Protector
19416, -- Ancient Draenei Spirit
19464, -- Bleeding Hollow Soul
19516, -- Void Reaver
19543, -- Battle-Mage Dathric
19544, -- Conjurer Luminrath
19545, -- Cohlien Frostweaver
19546, -- Abjurist Belmara
19749, -- Shadowmoon Specter
19751, -- Vengeful Shadowmoon Wraith onwards
19825, -- Dark Conclave Talonite
19826, -- Dark Conclave Shadowmancer
19827, -- Dark Conclave Ravenguard
19864, -- Vengeful Unyielding Captain
19872, -- Lady Catriona Von'Indi
19873, -- Lord Crispin Ference
19874, -- Baron Rafe Dreuger
19875, -- Baroness Dorothea Millstipe
19876, -- Lord Robin Daris
19881, -- Severed Spirit
20117, -- Vengeful Unyielding Knight
20137, -- Vengeful Unyielding Footman
20409, -- Kirin'Var Apprentice
20410, -- Rhonsus
20480, -- Kirin'Var Ghost
20496, -- Kirin'Var Spectre
20512, -- Tormented Soul
20934, -- Severed Defender
21058, -- Disembodied Exarch
21065, -- Tormented Citizen
21093, -- Dancing Sword
21198, -- Deathtalon Spirit
21200, -- Screeching Spirit
21324, -- Spirit Raven
21384, -- Dark Conclave Harbinger
21385, -- Dark Conclave Scorncrow
21386, -- Dark Conclave Hawkeye
21430, -- Unliving Draenei
21446, -- Bladespire Evil Spirit
21449, -- Cursed Spirit
21450, -- Skethyl Owl
21452, -- Bloodmaul Evil Spirit
21628, -- Highborne Lamenter
21636, -- Vengeful Draenei
21638, -- Vengeful Harbinger
21651, -- Time-Lost Skettis Reaver
21763, -- Time-Lost Skettis Worshipper
21784, -- Ghostrider of Karabor
21787, -- Time-Lost Skettis High Priest
21788, -- Shadowmoon Zealot
21795, -- Shadowmoon Harbinger
21801, -- Vhel'kur
21815, -- Cleric of Karabor
21869, -- Unliving Guardian
21870, -- Unliving Initiate
21941, -- Accursed Apparition
22041, -- Corrupted Spectre[PH]
22138, -- Dark Conclave Ritualist
22226, -- Koi-Koi Spirit
22235, -- Evil Koi-Koi
22452, -- Reanimated Exarch
22454, -- Fel Spirit
22953, -- Wrathbone Flayer
23066, -- Talonpriest Ishaal
23067, -- Talonpriest Skizzik
23068, -- Talonpriest Zellek
23109, -- Vengeful Spirit
23193, -- Lordaeron Citizen (Jesse)
23371, -- Shadowmoon Fallen
23399, -- Suffering Soul Fragment
23401, -- Hungering Soul Fragment
23469, -- Enslaved Soul
23554, -- Risen Spirit
23786, -- Stonemaul Spirit
24246, -- Darkheart
24693,25542, -- Arcane Nightmare
24808,25546, -- Broken Sentinel
24695,25559 -- Nether Shade
);

-- ------------------------------
-- Remove Misc NPC Added in 2.4.2
-- ------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (27666); -- Ontuvo <Jewelcrafting Supplies> - Shattered Sun in Terrace of Light
UPDATE creature SET spawnMask=0 WHERE id IN (27667); -- Anwehu <Weapons & Armorsmith> - Shattered Sun in Terrace of Light
-- UPDATE creature SET spawnMask=0 WHERE id IN (27704); -- Horace Alder <Mage Trainer> - Theramore Isle
-- UPDATE creature SET spawnMask=0 WHERE id IN (27705); -- Lorrin Foxfire <Portal Trainer> - Swamp of Sorrows
UPDATE creature SET spawnMask=0 WHERE id IN (27711); -- Technician Halmaha <Engineering Supplies> - Shattrath City
-- UPDATE creature SET spawnMask=0 WHERE id IN (27703); -- Ysuria <Portal Trainer> - Theramore Isle

-- -----------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Temp Revamp Shaani (Quel'danas gem vendor) Vendor Items To Remove ([Quick Dawnstone], [Reckless Noble Topaz], [Forceful Talasite]) - Offered From Vendor In 2.4.2
-- -----------------------------------------------------------------------------------------------------------------------------------------------------------------
UPDATE creature_template SET VendorTemplateId=0 WHERE entry IN (25950);
DELETE FROM npc_vendor WHERE entry IN (25950);
insert into `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `condition_id`, `comments`) values
('25950','32227','0','0','1642','0','Crimson Spinel'),
('25950','32228','0','0','1642','0','Empyrean Sapphire'),
('25950','32229','0','0','1642','0','Lionseye'),
('25950','32230','0','0','1642','0','Shadowsong Amethyst'),
('25950','32231','0','0','1642','0','Pyrestone'),
('25950','32249','0','0','1642','0','Seaspray Emerald'),
('25950','35238','0','0','0','0','Design: Balanced Shadowsong Amethyst'),
('25950','35239','0','0','0','0','Design: Glowing Shadowsong Amethyst'),
('25950','35240','0','0','0','0','Design: Infused Shadowsong Amethyst'),
('25950','35241','0','0','0','0','Design: Royal Shadowsong Amethyst'),
('25950','35242','0','0','0','0','Design: Shifting Shadowsong Amethyst'),
('25950','35243','0','0','0','0','Design: Sovereign Shadowsong Amethyst'),
('25950','35244','0','0','0','0','Design: Bold Crimson Spinel'),
('25950','35245','0','0','0','0','Design: Bright Crimson Spinel'),
('25950','35246','0','0','0','0','Design: Delicate Crimson Spinel'),
('25950','35247','0','0','0','0','Design: Flashing Crimson Spinel'),
('25950','35248','0','0','0','0','Design: Runed Crimson Spinel'),
('25950','35249','0','0','0','0','Design: Subtle Crimson Spinel'),
('25950','35250','0','0','0','0','Design: Teardrop Crimson Spinel'),
('25950','35251','0','0','0','0','Design: Dazzling Seaspray Emerald'),
('25950','35252','0','0','0','0','Design: Enduring Seaspray Emerald'),
('25950','35253','0','0','0','0','Design: Jagged Seaspray Emerald'),
('25950','35254','0','0','0','0','Design: Radiant Seaspray Emerald'),
('25950','35255','0','0','0','0','Design: Brilliant Lionseye'),
('25950','35256','0','0','0','0','Design: Gleaming Lionseye'),
('25950','35257','0','0','0','0','Design: Great Lionseye'),
('25950','35258','0','0','0','0','Design: Mystic Lionseye'),
('25950','35259','0','0','0','0','Design: Rigid Lionseye'),
('25950','35260','0','0','0','0','Design: Smooth Lionseye'),
('25950','35261','0','0','0','0','Design: Thick Lionseye'),
('25950','35262','0','0','0','0','Design: Lustrous Empyrean Sapphire'),
('25950','35263','0','0','0','0','Design: Solid Empyrean Sapphire'),
('25950','35264','0','0','0','0','Design: Sparkling Empyrean Sapphire'),
('25950','35265','0','0','0','0','Design: Stormy Empyrean Sapphire'),
('25950','35266','0','0','0','0','Design: Glinting Pyrestone'),
('25950','35267','0','0','0','0','Design: Inscribed Pyrestone'),
('25950','35268','0','0','0','0','Design: Luminous Pyrestone'),
('25950','35269','0','0','0','0','Design: Potent Pyrestone'),
('25950','35270','0','0','0','0','Design: Veiled Pyrestone'),
('25950','35271','0','0','0','0','Design: Wicked Pyrestone'),
('25950','35766','0','0','0','0','Design: Steady Seaspray Emerald'),
('25950','35767','0','0','0','0','Design: Reckless Pyrestone'),
('25950','35768','0','0','0','0','Design: Quick Lionseye'),
('25950','35769','0','0','0','0','Design: Forceful Seaspray Emerald'),
('25950','37504','0','0','0','0','Design: Purified Shadowsong Amethyst');

-- ----------------------------------------------------------
-- Cloak of Swift Mending Had Required Level 0 Prior to 2.4.2
-- ----------------------------------------------------------
UPDATE item_template SET RequiredLevel=0 WHERE entry IN (34702); -- Cloak of Swift Mending

-- -----------------------------------------------------
-- Goblin Jumper Cables were a trinket Item Untill 2.4.2
-- ------------------------------------------------------
UPDATE item_template SET InventoryType = 12, ExtraFlags=ExtraFlags|2 WHERE entry IN (7148,18587);

-- ----------------------------------------------------------------------------
-- Item Enhancement Items Had Profession Skill Requirement To Use Prior To 2.4.2
-- ----------------------------------------------------------------------------
-- As of Patch 2.4.2, Blacksmithing is not required to apply the weapon chain to a weapon.
UPDATE item_template SET requiredskill = 164 WHERE entry = 6041; -- Steel Weapon Chain (6041)
UPDATE item_template SET requiredskill = 164 WHERE entry = 33185; -- Adamantite Weapon Chain (33185)

-- As of Patch 2.4.2, Blacksmithing is not required to apply the counterweight to a weapon.
UPDATE item_template SET requiredskill = 164 WHERE entry = 6043; -- Iron Counterweight (6043)

-- Patch 2.4.2 (2008-05-13): No longer requires Blackmithing skill to attach.
-- This item needed a Blacksmith with 215 skill to attach them to your boots.
UPDATE item_template SET requiredskill = 164 WHERE entry = 7969; -- Mithril Spurs (7969)

-- As of Patch 2.4.2, Blacksmithing is not required to apply the shield spike to a shield.
UPDATE item_template SET requiredskill = 164 WHERE entry = 6042; -- Iron Shield Spike (6042)
UPDATE item_template SET requiredskill = 164 WHERE entry = 7967; -- Mithril Shield Spike (7967)
UPDATE item_template SET requiredskill = 164 WHERE entry = 12645; -- Thorium Shield Spike (12645)
UPDATE item_template SET requiredskill = 164 WHERE entry = 23530; -- Felsteel Shield Spike (23530)

-- -----------------------------------------------------
-- Many Consortium Vendor Items Were Not BoP Until 2.4.2
-- -----------------------------------------------------
UPDATE item_template SET bonding=0 WHERE entry IN (22552); -- Formula: Enchant Weapon - Major Striking
UPDATE item_template SET bonding=0 WHERE entry IN (23134); -- Design: Delicate Blood Garnet
UPDATE item_template SET bonding=0 WHERE entry IN (23146); -- Design: Shifting Shadow Draenite
UPDATE item_template SET bonding=0 WHERE entry IN (23150); -- Design: Thick Golden Draenite
UPDATE item_template SET bonding=0 WHERE entry IN (23155); -- Design: Lustrous Azure Moonstone

-- ------------------------------------------------
-- Some Aldor Vendor Items Were Not BoP Until 2.4.2
-- ------------------------------------------------
UPDATE item_template SET bonding=0 WHERE entry IN (23145); -- Design: Royal Shadow Draenite
UPDATE item_template SET bonding=0 WHERE entry IN (23149); -- Design: Gleaming Golden Draenite

-- --------------------------------------------------------
-- Some Shattered Sun Vendor Items Were Not BoP Until 2.4.2
-- --------------------------------------------------------
UPDATE item_template SET bonding=0 WHERE entry IN (34780); -- Naaru Ration
UPDATE item_template SET bonding=0 WHERE entry IN (35505); -- Design: Ember Skyfire Diamond

-- -------------------------------------------------------------
-- Recipe: Flask of Chromatic Resistance Was Not BoP Until 2.4.2
-- -------------------------------------------------------------
UPDATE item_template SET bonding=0 WHERE entry IN (13522,31357); -- Recipe: Flask of Chromatic Resistance

-- -----------------------------------------------------
-- Patch 2.4.2 - Silent Nerf for Wretched Devourer 24960
-- -----------------------------------------------------
-- Prenerf Versions of Mana Tap 33483 & Nether Shock 35334 used by Wretched Devourer 24960
-- https://www.wowhead.com/npc=24960/wretched-devourer#comments:id=260523:reply=33711
-- "their mana tap was only taking 370 mana from me the other day, not 1.4k like before"
-- "silencing shock for ~1k" -> Should also deal like double the dmg, dont do that for now and unsure how that was done
UPDATE `spell_template` SET `EffectRealPointsPerLevel1` = 20, `EffectMultipleValue1` = 1 WHERE `id` = 33483; -- 5, 2
UPDATE `spell_template` SET `DurationIndex` = 28 WHERE `id` = 35334; -- 27 (3 to 5secs silence)

-- -----------------------------------------------------------------------------------------------------------------------------
-- Patch 2.4 - Elites in Blade's Edge Plateau Regions: These creatures now have an increased chance to drop rare depleted items.
-- -----------------------------------------------------------------------------------------------------------------------------
-- ToDo: Add other bosses here too when their loot templates are reworked
-- Note: It is very hard to know what the droprate was before nerf. Current best guess based on wowhead/wayback machine is pre-nerf 15% and post 2.4 20%.
UPDATE creature_loot_template SET ChanceOrQuestChance=15 WHERE ChanceOrQuestChance=20 AND minCountOrRef=-65279;

-- ----------------------------------------------------------
-- Lightning Wasp c.22182 removed in 2.4
-- http://wowwiki.wikia.com/wiki/Lightning_Wasp
-- ----------------------------------------------------------
DELETE FROM creature WHERE id IN (22182);
INSERT INTO creature (guid, id, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecsmin, spawntimesecsmax, spawndist, currentwaypoint, curhealth, curmana, DeathState, MovementType) VALUES
-- Lightning Wasp
(77792, 22182, 530, 1, 0, 0, 3681.75, 5952.07, 271, 2.63468, 300, 300, 5, 0, 6986, 0, 0, 1), -- z manually updated due to spawning below ground. Old value: 269.154
(77793, 22182, 530, 1, 0, 0, 3774.83, 5869.73, 266.125, 5.46492, 300, 300, 5, 0, 6986, 0, 0, 1),
(77794, 22182, 530, 1, 0, 0, 3799.72, 5901.78, 267.28, 5.84971, 300, 300, 5, 0, 6986, 0, 0, 1),
(77795, 22182, 530, 1, 0, 0, 3697.77, 5994.4, 265.155, 4.73437, 300, 300, 5, 0, 6986, 0, 0, 1),
(77796, 22182, 530, 1, 0, 0, 3736.45, 5989.69, 266.06, 3.22816, 300, 300, 5, 0, 6986, 0, 0, 1),
(77797, 22182, 530, 1, 0, 0, 3668.73, 6010.27, 267.269, 0.192801, 300, 300, 5, 0, 6986, 0, 0, 1),
(77798, 22182, 530, 1, 0, 0, 3695.98, 6046.25, 267.102, 1.86269, 300, 300, 5, 0, 6986, 0, 0, 1),
(77799, 22182, 530, 1, 0, 0, 3802.79, 6008.81, 265.166, 4.4149, 300, 300, 5, 0, 6986, 0, 0, 1),
(77800, 22182, 530, 1, 0, 0, 3668.35, 6099.08, 267.089, 4.44155, 300, 300, 5, 0, 6986, 0, 0, 1),
(77801, 22182, 530, 1, 0, 0, 3653.41, 6129.36, 276, 4.92939, 300, 300, 5, 0, 6986, 0, 0, 1), -- z manually updated due to spawning below ground. Old value: 269.067
(77802, 22182, 530, 1, 0, 0, 3625.29, 6151.41, 273.4354, 0.028046, 300, 300, 5, 0, 6986, 0, 0, 1),
(77803, 22182, 530, 1, 0, 0, 3647.64, 6185.27, 273.5, 0.337214, 300, 300, 5, 0, 6986, 0, 0, 1),-- z manually updated due to spawning below ground. Old value: 271.207
(77804, 22182, 530, 1, 0, 0, 3850.14, 5930.05, 267.726, 5.5877, 300, 300, 5, 0, 6986, 0, 0, 1),
(77805, 22182, 530, 1, 0, 0, 3885.45, 5909.33, 266.545, 2.9837, 300, 300, 5, 0, 6986, 0, 0, 1),
(77806, 22182, 530, 1, 0, 0, 3939.66, 6058.46, 266.583, 1.72858, 300, 300, 5, 0, 6986, 0, 0, 1),
(77807, 22182, 530, 1, 0, 0, 3898.78, 6034.49, 266.286, 0.119334, 300, 300, 5, 0, 6986, 0, 0, 1),
(77808, 22182, 530, 1, 0, 0, 3909.11, 6087.4, 271, 4.62775, 300, 300, 5, 0, 6986, 0, 0, 1), -- z manually updated due to spawning below ground. Old value: 266.334
(77809, 22182, 530, 1, 0, 0, 3856.18, 6075.34, 265.568, 5.05943, 300, 300, 5, 0, 6986, 0, 0, 1),
(77810, 22182, 530, 1, 0, 0, 3775.31, 6126.39, 267.4, 3.43915, 300, 300, 5, 0, 6986, 0, 0, 1),
(77811, 22182, 530, 1, 0, 0, 3833, 6134.83, 265.948, 5.41649, 300, 300, 5, 0, 6986, 0, 0, 1),
(77812, 22182, 530, 1, 0, 0, 3742.8, 6188.77, 265.066, 6.17314, 300, 300, 5, 0, 6986, 0, 0, 1),
(77813, 22182, 530, 1, 0, 0, 3711.6, 6204.93, 265.664, 0.214292, 300, 300, 5, 0, 6986, 0, 0, 1);

-- -------------------------------------------------------------------------------------- 
-- Despawn all Shattered Sun Offensive NPCS, Unavailable before Quel'Danas release
-- --------------------------------------------------------------------------------------
UPDATE creature SET spawnmask=0 WHERE id IN(SELECT entry FROM creature_template WHERE Faction IN(1956,1957,1960,1967));
UPDATE creature SET spawnmask=0 WHERE id = 25147;

-- Brilliant Glass
-- Patch 2.4.0 (25-Mar-2008): Added.
DELETE FROM npc_trainer_template WHERE spell=47280;
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry=35945;

-- Now That We're Friends... (and follow-up Now That We're Still Friends...)
-- These quests required 12 Bloodscale Enchantress to be slain instead of 6 prior to patch 2.4
-- "Several repeatable quests have been nerfed in patch 2.4. The number of Enchantresses you need to kill is really 6 now."
-- "I think this changed with 2.4 - Prior to 2.4 it was 12 Enchantresses and now it is only 6."
UPDATE quest_template SET ReqCreatureOrGOCount2=12, Details=REPLACE(Details,'6 Bloodscale Enchantresses','12 Bloodscale Enchantresses'), Objectives=REPLACE(Objectives,'6 Bloodscale Enchantresses','12 Bloodscale Enchantresses') WHERE entry IN (9726,9727);

-- Patch 2.4.0 (25-Mar-2008): Ancient Lichen now has a chance to drop a Fel Lotus where it used to drop a piece of random green jewelry.
-- https://web.archive.org/web/20111112145322/http://www.wowhead.com/item=22794#gathered-from-object
DELETE FROM `gameobject_loot_template` WHERE `entry` = 18116 AND `item` IN (22794,11979,11980,11992,12005,12016,12017,12036,12048,12058,12027);
INSERT INTO `gameobject_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES
(18116, 11979, 0.5, 1, 1, 1, 0, 'Peridot Circle'),
(18116, 11980, 0.5, 1, 1, 1, 0, 'Opal Ring'),
(18116, 11992, 0.5, 1, 1, 1, 0, 'Vermilion Bande'),
(18116, 12005, 0.5, 1, 1, 1, 0, 'Granite Ring'),
(18116, 12016, 0.5, 1, 1, 1, 0, 'Jungle Ring'),
(18116, 12017, 0.5, 1, 1, 1, 0, 'Prismatic Band'),
(18116, 12036, 0.5, 1, 1, 1, 0, 'Granite Necklace'),
(18116, 12048, 0.5, 1, 1, 1, 0, 'Prismatic Pendant'),
(18116, 12058, 0.5, 1, 1, 1, 0, 'Demonic Bone Ring'),
(18116, 12027, 0.5, 1, 1, 1, 0, 'Vermilion Necklace');

-- Patch 2.4.0 (2008-03-25): Now wears a Shattered Sun Offensive tabard, as part of the launching of the Battle for Quel'Danas.
UPDATE `creature` SET `position_x` = -1713.21, `position_y` = 5646.24, `position_z` = 128.023, `orientation` = 3.41571, `modelid` = 18741 WHERE `id` = 25195;
UPDATE `creature_template_addon` SET `sheath_state` = 0 WHERE `entry` = 25195;

-- =====================
-- Misc Raid Adjustments
-- =====================

-- Nalorakk and Mother Shahraz will no longer haste their attacks following a parry. - Feb. 23, 2008, 12:57 a.m.
-- https://www.bluetracker.gg/wow/topic/us-en/4823644205-nalorakk-and-mother-shahraz-parry-fix/
-- https://wowwiki-archive.fandom.com/wiki/Mother_Shahraz?diff=prev&oldid=1474542
UPDATE `creature_template` SET `ExtraFlags` = `ExtraFlags`&~8 WHERE `entry` IN (22947,23576);

-- Patch 2.4 - Azgalor's Rain of Fire now affects a smaller area. - Rain of Fire 31340 - (20 -> 15)
-- maybe readd when method to increase animation radius is found, like this only the debuff shows that the player is still within the radius, which for prenerf emulation would be enough but currently has not to be used to allow easier progress
-- UPDATE `spell_template` SET `EffectRadiusIndex1` = 9, `EffectRadiusIndex2` = 9 WHERE `Id` = 31340;

-- Patch 2.3.2 - The melee haste provided by Halazzi's Frenzy has been reduced to 100%.
UPDATE `spell_template` SET `EffectBasePoints1` = 149 WHERE `Id` = 43139;

-- Patch 2.3.2 - The Amani'shi Warrior's Charge ability now has a minimum range
UPDATE `spell_template` SET `RangeIndex` = 34 WHERE `Id` = 43519; -- 95 (8-25)
DELETE FROM `creature_ai_scripts` WHERE `id` = 2422501; -- Need to Adjust ACID currently, as it uses Event 9 (Range)
INSERT INTO creature_ai_scripts (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2422501','24225','0','0','100','1025','0','5000','12000','24000','0','0','11','43519','1','0','0','0','0','0','0','0','0','0','Amani''shi Warrior - Cast Charge');

-- ========================
-- Misc Dungeon Adjustments
-- ========================



-- ======================================
-- Custom Changes (unconfirmed or custom)
-- ======================================

-- Reduce Dropchance for Warglaive of Azzinoth to 3% each from 4% (2.4.3)
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 3 WHERE `entry` = 22917 AND `item` IN (32837,32838);

-- Add Enrage 8599 for Amani'shi Protector 24180 Added in Cataclysm Rework
DELETE FROM `creature_ai_scripts` WHERE `id` = 2358001;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2418005','24180','2','0','100','1024','25','0','0','0','0','0','11','8599','0','2','1','-46','0','0','0','0','0','0','Amani''shi Protector - Cast Enrage at 25% HP');

-- Make Amani''shi Flame Caster somewhat more threatening
DELETE FROM `creature_ai_scripts` WHERE `id` = 2359603;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2359603','23596','0','0','100','1025','3000','6000','10000','15000','0','0','11','43240','1','0','0','0','0','0','0','0','0','0','Amani''shi Flame Caster - Cast Fireball Volley');

-- Moonwell
alter table creature drop column modelid;
alter table creature drop column equipment_id;
alter table creature drop column currentwaypoint;
alter table creature drop column curhealth;
alter table creature drop column curmana;
alter table creature drop column DeathState;