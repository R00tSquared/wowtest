-- This file should only be run BEFORE 2.4.3 is released

-- ============================================================================================================================================
-- ==========         Patch Release 2.4.1 - 2.4.3 Content Reversions - End of Content Final Release To Finish at Patch 2.4.3         ==========
-- ==========               (Complete Post 2.4.0 to 2.4.3 Content Reversions As Per Blizz Patch Notes and Wowhead Data)              ==========
-- ============================================================================================================================================

-- ===========================
-- Patch 2.4.1 Content Changes
-- ===========================

-- https://tbc.wowhead.com/npc=2943/ransin-donner#comments:id=324824
-- ~2008/07/12 (Patch 2.4.1) Ransin Donner 2943 - He no longer wears his murloc costume.
UPDATE `creature_template` SET `modelid1`='21770' WHERE `entry`='2943'; -- Ransin Donner
-- https://tbc.wowhead.com/npc=7951/zastysh#comments:id=2807049
-- ~2008/05/20 (Patch 2.4.1) Zas'Tysh 7951 - NPC is no longer in the murloc costume since the last patch.
UPDATE `creature_template` SET `modelid1`='21769' WHERE `entry`='7951'; -- Zas'Tysh

-- ===========================
-- Patch 2.4.3 Content Changes
-- ===========================
UPDATE `creature` SET `position_x` = -8908.31, `position_y` = -108.521, `position_z` = 81.9314, `orientation` = 4.13643 WHERE `id` = 11940; -- Merissa Stilwell - Patch 2.4.3 (2008-07-15): Moved from Northshire Abbey to Goldshire.
UPDATE `creature` SET `position_x` = -6230.39, `position_y` = 320.446, `position_z` = 383.144, `orientation` = 0.925025 WHERE `id` = 11941; -- Yori Crackhelm - Patch 2.4.3 (2008-07-15): Moved from Anvilmar to Kharanos.
UPDATE `creature` SET `position_x` = 10324.1, `position_y` = 820.373, `position_z` = 1326.51, `orientation` = 1.55334 WHERE `id` = 11942; -- Orenthil Whisperwind - Patch 2.4.3 (2008-07-15): Moved from Shadowglen to Dolanaar.
UPDATE `creature` SET `position_x` = -638.159, `position_y` = -4237.94, `position_z` = 38.2173, `orientation` = 0.017453 WHERE `id` = 11943; -- Magga - Patch 2.4.3 (2008-07-15):  Moved from Valley of Trials to Razor Hill.
UPDATE `creature` SET `position_x` = -2909.55, `position_y` = -231.807, `position_z` = 53.9186, `orientation` = 5.16617 WHERE `id` = 11944; -- Vorn Skyseer - Patch 2.4.3 (2008-07-15): Moved from Camp Narache to Bloodhoof Village.
UPDATE `creature` SET `position_x` = 1834.02, `position_y` = 1581.34, `position_z` = 95.1515, `orientation` = 2.11185 WHERE `id` = 11945; -- Claire Willower - Patch 2.4.3 (2008-07-15): Moved from Deathknell to Brill.

-- ---------------------------------------------
-- Jaeleil Was Relocated To Azure Watch in 2.4.3 - This Is Original Old Location
-- ---------------------------------------------
UPDATE creature SET position_x = -4083.05, position_y = -13757.8, position_z = 74.8751, orientation = 5.77704, MovementType = 0, spawndist = 0 WHERE id = 16476;
UPDATE creature_template_addon SET stand_state = 0 WHERE entry = 16476;

-- -------------------------------------------------------------
-- Apprentice Riding Trainer Cost and Level Was Changed in 2.4.3 - (Riding Was Dropped From Level 40 to 30 and The Cost Was Reduced)
-- -------------------------------------------------------------
UPDATE npc_trainer SET spellcost=900000, ReqLevel=40 WHERE spell IN (33388); -- Apprentice Trainer
UPDATE npc_trainer_template SET spellcost=900000, ReqLevel=40 WHERE spell IN (33388); -- Apprentice Trainer
DELETE FROM mail_level_reward WHERE MailTemplateId BETWEEN 224 AND 233; -- Mail Sent Out When Player Reaches Proper Level
UPDATE npc_trainer_template SET ReqLevel=40 WHERE spell IN (5784,13819,34769); -- Set Level 40 Requirement
UPDATE item_template SET RequiredLevel=40, ItemLevel=40 WHERE RequiredLevel=30 AND `class`=15; -- 37 mount items impacted

-- ---------------------------------------------------------
-- Harris Pilton Vendor Items Were Not Available Until 2.4.3
-- ---------------------------------------------------------
UPDATE creature_template SET Faction = 1818, npcflags=2 WHERE entry IN (18756);
DELETE FROM npc_vendor WHERE entry IN (18756);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (37934,38082,38090,38091);

-- ------------------------------------------------
-- Nether Ray Fry Pet was not available until 2.4.3
-- ------------------------------------------------
DELETE FROM npc_vendor WHERE item IN (38628); -- Nether Ray Fry
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (38628);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------
-- Blood Elf flightmasters outside of Silvermoon City and Tranquillien have traded in their bats for glorious fire-breathing dragonhawks in Patch 2.4.3
-- ----------------------------------------------------------------------------------------------------------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (27946); -- Silvermoon Dragonhawks

-- ------------------------------------------------------------
-- Philosopher's Stone Lowered From 225 to 200 Alchemy In 2.4.3
-- ------------------------------------------------------------
UPDATE item_template SET RequiredSkillRank=225 WHERE entry IN (9149);

-- -------------------------------------
-- New Patterns That Were Added In 2.4.3
-- -------------------------------------
DELETE FROM npc_vendor WHERE item IN (38327); -- Pattern: Haliscan Jacket
DELETE FROM npc_vendor WHERE item IN (38328); -- Pattern: Haliscan Pantaloons
DELETE FROM npc_vendor WHERE item IN (38229); -- Pattern: Mycah's Botanical Bag
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (38327,38328,38229);

-- -------------------------------------------------
-- Bhag'thera Had Longer Respawn Time Prior to 2.4.3
-- -------------------------------------------------
UPDATE creature SET SpawnTimeSecsMin=180,SpawnTimeSecsMax=180 WHERE id IN (728);

-- ------------------------------------------------------------------------------------
-- Winterspring Ice Thistle Matriarchs and Patriarchs drop [Thick Yeti Fur] as of 2.4.3
-- ------------------------------------------------------------------------------------
-- DELETE FROM creature_loot_template WHERE entry IN (7459,7460) AND item IN (12366);

-- ------------------------------------------------------------------------------------------
-- Skinning any Winterspring Ice Thistle yetis may now result in [Thick Yeti Fur] as of 2.4.3
-- ------------------------------------------------------------------------------------------
-- DELETE FROM creature_loot_template WHERE entry IN (7457,7458,7459,7460) AND item IN (12366);

-- ---------------------------------------------------------------------------------------------------
-- Ice Thistle Matriarchs and Patriarchs Had a Lower Drop Rate of [Pristine Yeti Horns] Prior to 2.4.3
-- ---------------------------------------------------------------------------------------------------
-- UPDATE creature_loot_template SET ChanceOrQuestChance=-40 WHERE entry IN (7459,7460) AND item IN (12367);

-- -----------------------------------------------------------------------------------------------------------------------------------
-- Young SporeBats and Greater SporeBats in Zangarmarsh now drop [Sporebat Eyes] for the quest Gathering the Reagents (Added in 2.4.3)
-- -----------------------------------------------------------------------------------------------------------------------------------
-- DELETE FROM creature_loot_template WHERE entry IN (18129,20387) AND item IN (24426);

-- ---------------------------------------------------------------------------------
-- Remove Blazzle (Blacksmith Vendor) and Meeda (Banker) in Area 52 (Added in 2.4.3)
-- ---------------------------------------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (28344); -- Blazzle
UPDATE creature SET spawnMask=0 WHERE id IN (28343); -- Meeda

-- ------------------------------
-- Remove Misc NPC Added in 2.4.3
-- ------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (28067); -- Dark Iron Brewer
UPDATE creature SET spawnMask=0 WHERE id IN (28126); -- Don Carlos (Gadetzan)
UPDATE creature SET spawnMask=0 WHERE id = 5403 AND guid = 65613; -- Don Carlos - White Stallion (Gadetzan)
UPDATE creature SET spawnMask=0 WHERE id IN (28132); -- Don Carlos (Old Hillsbrad Foothills)
UPDATE creature SET spawnMask=0 WHERE id IN (28163); -- Guerrero (Old Hillsbrad Foothills)
UPDATE creature SET spawnMask=0 WHERE id IN (28225); -- Griz Gutshank <Arena Vendor>
UPDATE creature SET spawnMask=0 WHERE id IN (26081); -- High Admiral "Shelly" Jorrik
UPDATE creature SET spawnMask=0 WHERE id IN (28209); -- Mizli Crankwheel
UPDATE creature SET spawnMask=0 WHERE id IN (28210); -- Ognip Blastbolt
UPDATE creature SET spawnMask=0 WHERE id IN (27398); -- Gilbarta Grandhammer <Battleground Enthusiast> - Part Of Special Event During 2008 Bejing Olympics

-- -------------------------------------------------
-- Remove Stormwind Contruction Crews Added in 2.4.3
-- -------------------------------------------------
UPDATE creature SET spawnMask=0 WHERE id IN (28571); -- Foreman Wick (Stormwind Construction Crew)
UPDATE creature SET spawnMask=0 WHERE id IN (28572); -- Mason Goldgild (Stormwind Construction Crew)
UPDATE creature SET spawnMask=0 WHERE id IN (28569); -- Construction Worker (Stormwind Construction Crew)
UPDATE creature SET spawnMask=0 WHERE id IN (28596); -- Dwarven Construction Worker (Stormwind Construction Crew)
UPDATE creature SET spawnMask=0 WHERE id IN (28573); -- Underwater Construction Worker (Stormwind Construction Crew)

-- Remove Construction GO's
UPDATE gameobject SET spawnMask=0 WHERE id IN (190571);

-- ----------------------------------------
-- Surestrike level requirment added in 2.4
-- -----------------------------------------
UPDATE item_template SET requiredLevel = 0 WHERE Entry = 32474;

-- --------------------------------------------------------------------------
-- This NPC was renamed from "Kristen DeMeza" to "Kristen Dipswitch" in 2.4.3
-- --------------------------------------------------------------------------
UPDATE creature_template SET Name='Kristen DeMeza' WHERE entry=18294;

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Durn the Hungerer - Shoulder Charge
-- It is unknown when this spell was removed from his spellpool, therefore I'm adding this change here until we know if it lasted during the whole duration of TBC or not.
-- Evidence used:
-- * https://www.youtube.com/watch?v=saFZxhalxTY
-- * https://imgur.com/a/YhHil4M
-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------
DELETE FROM creature_ai_scripts WHERE creature_id=18411 AND id=1841103;
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('1841103','18411','32','0','100','1025','8','25','12000','30000','11','31994','12','0','0','0','0','0','0','0','0','0','Durn the Hungerer - Cast Shoulder Charge in Range');

-- Brilliant Glass
-- Patch 2.4.2 (2008-05-13): No longer requires a forge. Now has a chance to drop an epic gem.
DELETE FROM item_loot_template WHERE item IN (32227,32228,32229,32230,32231,32249) AND entry=35945;
UPDATE spell_template SET RequiresSpellFocus=3 WHERE Id=47280;

-- =====================
-- Misc Raid Adjustments
-- =====================

-- Hellfire Channeler (17256) - Soul Transfer
-- 2.4.3: damage +20%, casting speed -10%
-- pre-nerf: damage +30%, casting speed -30%
UPDATE spell_template SET EffectBasePoints1=29, EffectBasePoints2=29 WHERE id=30531; -- 19, 9

-- increase duration of Mind Exhaustion to 90 seconds (Patch 2.4) 30secs (9) 2.4.3
UPDATE spell_template SET DurationIndex=23 WHERE id IN(44032);

-- Add Deep Wounds 23255 & Dual Wield (Passive) 42459 - potential cata zul'aman version only
UPDATE `creature_template_addon` SET `auras` = '23255 42459' WHERE `entry` = 23580; -- Amani'shi Warbringer
UPDATE `creature_template_addon` SET `auras` = '42459' WHERE `entry` IN (
23577, -- Halazzi
23578, -- Jan'alai
23582, -- Amani'shi Tribesman
24549 -- Amani'shi Tempest
);

-- Amani'shi Guardian 23597 has Enrage Ability in Cata, add as Prenerf Ability
DELETE FROM `creature_ai_scripts` WHERE `id` IN (2359704);
INSERT INTO `creature_ai_scripts` (`id`, `creature_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action1_type`, `action1_param1`, `action1_param2`, `action1_param3`, `action2_type`, `action2_param1`, `action2_param2`, `action2_param3`, `action3_type`, `action3_param1`, `action3_param2`, `action3_param3`, `comment`) VALUES
('2359704','23597','2','0','100','1024','25','0','0','0','0','0','11','8599','0','34','0','0','0','0','0','0','0','0','Amani''shi Protector - Cast Enrage at 25% HP');

-- Patch 2.4.3 - Sinister Reflections are now interruptible.
UPDATE `creature_template` SET `MechanicImmuneMask` = `MechanicImmuneMask`|33554432 WHERE `entry` = 25708;

-- ========================
-- Misc Dungeon Adjustments
-- ========================

-- https://de.wowhead.com/npc=17800/myrmidone-des-echsenkessels#comments:id=258895
-- Coilfang Myrmidon 17800 should have a sweeping strikes ability apart from it's cleave ability which made it so deadly - spellid guessed, maybe its direct execute on random target or something
DELETE FROM `creature_ai_scripts` WHERE `id` IN (1780004);
INSERT INTO `creature_ai_scripts` (`id`, `creature_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action1_type`, `action1_param1`, `action1_param2`, `action1_param3`, `action2_type`, `action2_param1`, `action2_param2`, `action2_param3`, `action3_type`, `action3_param1`, `action3_param2`, `action3_param3`, `comment`) VALUES
('1780004','17800','0','0','100','1025','2000','20000','20000','40000','0','0','11','35429','0','0','0','0','0','0','0','0','0','0','Coilfang Myrmidon - Cast Sweeping Strikes');

-- Patch 2.4.3 - The Stun component to the Sunblade Mage Guard Glaive Throw has been removed and the Bounce range reduced.
-- Readd Stun Component for Sunblade Mage Guard 24683 - Glaive Throw 44478,46028
UPDATE `spell_template` SET `DurationIndex` = 39, `Effect2` = 6, `EffectApplyAuraName2` = 12, `EffectImplicitTargetA2` = 6, `EffectChainTarget2` = 5 WHERE `Id` IN (44478,46028);

-- Patch 2.4.3 - Vexallus' damage caused by Pure Energy has been decreased.

-- Vexallus (Normal) Mode Was Immune To Taunt Until 2.4.2 and (Heroic) Was Immune To Taunt Until 2.4.3
UPDATE `creature_template` SET ExtraFlags=ExtraFlags|256 WHERE entry IN (24744,25573);

-- Kael'thas Sunstrider (Normal) Mode Was Immune To Taunt Until 2.4.2 and (Heroic) Was Immune To Taunt Until 2.4.3
UPDATE `creature_template` SET ExtraFlags=ExtraFlags|256 WHERE entry IN (24664,24857);

-- Patch 2.4.3 - Kael'thas' Arcane Sphere has had its visual size increased. His Arcane Sphere attack has had its range decreased and the damage/second of Phase two on Normal mode has been decreased. (Heroic mode remains unchanged.)

