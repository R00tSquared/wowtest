-- This file should only be run BEFORE 2.2.0 is released
-- https://wow.gamepedia.com/Patch_2.2.0

-- Graveyard changes - Patch 2.2.2
UPDATE creature SET spawnMask=0 WHERE guid = 5724 AND id=6491; -- The Barrens
UPDATE creature SET spawnMask=0 WHERE guid = 6858 AND id=6491; -- Western Plaguelands
UPDATE creature SET spawnMask=0 WHERE guid = 5725 AND id=6491; -- Alterac Mountains
UPDATE creature SET spawnMask=0 WHERE guid = 5729 AND id=6491; -- Searing Gorge
UPDATE creature SET spawnMask=0 WHERE guid = 6875 AND id=6491; -- Badlands
UPDATE creature SET spawnMask=0 WHERE guid IN (6849,5715) AND id=6491; -- Tanaris
UPDATE creature SET spawnMask=0 WHERE guid IN (6876,5752) AND id=6491; -- Winterspring
UPDATE creature SET spawnMask=0 WHERE guid = 5717 AND id=6491; -- Stonetalon Mountain
UPDATE creature SET spawnMask=0 WHERE guid IN (5749,7715) AND id=6491; -- Un'Goro Crater

-- ========================
-- Items/Loot
-- ========================

-- Patch 2.2.0 - Tempest Keep and Coilfang raid bosses will now all drop at least 3 items each. "non-tier token bosses in T5 only dropped 2 items instead of 3"
-- Al'ar
UPDATE reference_loot_template SET entry=36000 WHERE item IN (29922,29920);
UPDATE reference_loot_template SET entry=36001 WHERE item IN (29925,29924);
DELETE FROM creature_loot_template WHERE entry=19514 AND item=36002;
-- High Astromancer Solarian
UPDATE reference_loot_template SET entry=36005 WHERE item IN (29962,29981);
UPDATE reference_loot_template SET entry=36006 WHERE item IN (30446,30449);
DELETE FROM creature_loot_template WHERE entry=18805 AND item=36007;
-- Hydross
UPDATE reference_loot_template SET entry=36011 WHERE item IN (30049,30050,30051);
UPDATE reference_loot_template SET entry=36012 WHERE item IN (30053,30664,32516);
DELETE FROM creature_loot_template WHERE entry=21216 AND item=36013;
-- Lurker
UPDATE reference_loot_template SET entry=36014 WHERE item IN (30058,30059);
UPDATE reference_loot_template SET entry=36015 WHERE item IN (30065,30066);
DELETE FROM creature_loot_template WHERE entry=21217 AND item=36016;
-- Morogrim Tidewalker
UPDATE reference_loot_template SET entry=36017 WHERE item IN (30008,30068);
UPDATE reference_loot_template SET entry=36018 WHERE item IN (30075,30079,30080);
DELETE FROM creature_loot_template WHERE entry=21213 AND item=36019;

-- Heart of Darkness & Mark of the Illidari did only drop in Black Temple prior to Patch 2.2
-- https://tbc.wowhead.com/item=32428/heart-of-darkness#comments:id=165108
-- https://tbc.wowhead.com/item=32897/mark-of-the-illidari#comments:id=158640
-- https://tbc.wowhead.com/item=32897/mark-of-the-illidari#comments:id=5255718
DELETE FROM `creature_loot_template` WHERE `entry` NOT IN (
22844,22845,22846,22847,22853,22855,22869,22873,22874,22875,22876,22877,22878,22879,
22880,22881,22882,22883,22884,22885,22939,22945,22946,22953,22954,22955,22956,22957,
22959,22962,22964,22965,23018,23028,23030,23047,23049,23147,23172,23196,23222,23223,
23232,23235,23236,23237,23239,23330,23337,23339,23374,23394,23397,23400,23402,23403) AND `item` IN (32428,32897);

-- Patch 2.2.0 (2007-09-25): Now stacks to ten.
UPDATE item_template SET stackable = 5 WHERE entry = 17030; -- Ankh

UPDATE item_template SET requireddisenchantskill = -1, DisenchantID = 0 WHERE entry = 28108; -- Power Infused Mushroom
DELETE FROM npc_vendor WHERE Entry = 18255 AND item IN (33124,33165,33205,33209); -- Violet Eye Patterns
DELETE FROM creature_loot_template WHERE item = 33174; -- Removed Plans: Ragesteel Shoulders from loot table
DELETE FROM npc_vendor WHERE entry = 21655 AND item = 33148; -- Formula: Enchant Cloak Dodge
DELETE FROM npc_vendor WHERE entry IN (17657,17585) AND item = 20735; -- Formula: Enchant Cloak - Subtlety
DELETE FROM npc_vendor WHERE entry = 21643 AND item = 20731; -- Formula: Enchant Gloves - Superior Agility
DELETE FROM npc_vendor WHERE entry = 21432 AND item = 33153; -- Formula: Enchant Gloves - Threat
DELETE FROM npc_vendor_template WHERE entry = 511 AND item = 33783; -- Design: Steady Talasite

-- Consortium
-- Design: Crimson Sun, Design: Don Julio's Heart
DELETE FROM npc_vendor_template WHERE item IN (33156,33305);

-- Sha'tar
-- Design: Blood of Amber, Design: Kailee's Rose
DELETE FROM npc_vendor WHERE item IN (33159,33155);

-- Keepers of Time
-- Design: Facet of Eternity, Design: Stone of Blades
DELETE FROM npc_vendor WHERE item IN (33160,33158);

-- Lower City
-- Design: Falling Star
DELETE FROM npc_vendor WHERE item = 33157;

-- Cenarion Expedition
-- Formula: Enchant Cloak - Stealth
DELETE FROM npc_vendor WHERE item = 33149;

-- Nexus Transformation and Small Prismatic Shards were not taught by Enchanting trainers until 2.2
DELETE FROM npc_trainer_template WHERE spell IN (42613,42615);

-- Kurzen Jungle Fighter - apparently did not drop Jungle Remedy until 2.2?
DELETE FROM creature_loot_template WHERE entry = 937 AND item = 2633;

-- Mana Potion Injectors Changed in 2.2.0 (As of Patch 2.2, you create a stack of 20 Mana Potion Injectors with one charge each, instead of one Mana Potion Injector with 20 charges.)
-- http://www.wowhead.com/item=23823/mana-potion-injector OLD
-- https://www.wowhead.com/item=33093/mana-potion-injector NEW
UPDATE spell_template SET EffectItemType1=23823, EffectBasePoints1=1, AttributesEx=0, SpellVisual=395 WHERE Id=30552;
-- Same for Healing Potion Injectors
-- https://www.wowhead.com/?item=23822 OLD
-- https://www.wowhead.com/item=33092/healing-potion-injector NEW
UPDATE spell_template SET EffectItemType1=23822, EffectBasePoints1=1, AttributesEx=0, SpellVisual=395 WHERE Id=30551;
-- Remove the item flavor text, added in 2.2 when the old items were deprecated
UPDATE item_template SET description='' WHERE entry IN(23823,23822);

-- Markaru: This creature is now skinnable.
-- Void Terror: This creature is now skinnable.
UPDATE `creature_template` SET `SkinningLootId` = 0 WHERE `entry` IN (19980,20775);
DELETE FROM `skinning_loot_template` WHERE `entry` IN (20775);

-- Prior to patch 2.2, Hellfire Shot and Felbane Slugs were not BoP
UPDATE item_template SET bonding=0 WHERE entry IN (32882,32883);

-- Blade\'s Edge Mountains (Boss Loot) - Jewelcrafting Design 25030
DELETE FROM `creature_loot_template` WHERE `mincountOrRef` = -25030; -- i.31870,31871,31872,31873,31874

-- [Design: Great Golden Draenite]
-- Item ID: 31870
-- http://www.wowhead.com/item=31870/design-rigid-azure-moonstone#comments
-- fariseo on 2007/09/30 (Patch 2.2.0):
-- "Just dropped for me off Shartuul so we confirm that the design has been introduced in the game with the 2.2 patch"
DELETE FROM reference_loot_template WHERE item IN(31870);

-- [Design: Balanced Shadow Draenite]
-- Item ID: 31871
-- By Jellibean on 2007/09/28 (Patch 2.2.0):
-- "This dropped from an Apexis Guardian in Blade's Edge while doing the Guardian of the Monument quest. Happened the day after 2.2 went live, so I don't know if it's a new addition but I don't know of anyone with this particular design up to this point."
DELETE FROM reference_loot_template WHERE item IN(31871);

-- [Design: Infused Shadow Draenite]
-- Item ID: 31872
-- I can't find any evidence for this design, but it seems to be another new design from the mobs in Blade's Edge Mountains (like the designs above). Added in patch 2.2.0.
DELETE FROM reference_loot_template WHERE item IN(31872);

-- [Design: Veiled Flame Spessarite]
-- Item ID: 31873
-- http://www.wowhead.com/item=31873/design-veiled-shadow-draenite#comments
-- By Jops on 2007/09/27 (Patch 2.2.0):
-- "This is added in the game as of patch 2.2"
DELETE FROM reference_loot_template WHERE item IN(31873);

-- [Design: Wicked Flame Spessarite]
-- Item ID: 31874
-- http://www.wowhead.com/item=31874/design-deadly-flame-spessarite#comments
-- By ealbright on 2007/10/02 (Patch 2.2.2):
-- "I had this drop off one of the summoned demons near Ogri'la twice... I think it was Mo'arg Incinerator. All 5 of the new uncommon JC designs appear to be dropping off of the summonable BEM elites"
-- This seems to be another 2.2.0 design from the Blade's Edge Mountains mobs (notice the "All 5 of the new uncommon JC designs".
DELETE FROM reference_loot_template WHERE item IN(31874);

-- ------------------------------------------
-- Exalted Enchanting Formulas added in 2.2.0
-- Prior to 2.2.0 they had to be obtained the classic way (from drops in AQ)
-- ------------------------------------------
-- https://www.wowhead.com/item=33150/formula-enchant-cloak-subtlety#comments
-- By Viper007Bond (17,246 – 3·22·137) on 2007/08/01 (Patch 2.1.3)		
-- Won't be available until 2.2: http://www.mmo-champion.com/index.php?topic=470.0
-- By d3nalii (5,583 – 2·16·75) on 2007/09/26 (Patch 2.1.3)		
-- Changed, in 2.2.0 - now sold at the Thrallmar (or Honor Hold) vendor. You must have Exalted faction with them to buy it.
-- https://www.wowhead.com/item=33148/formula-enchant-cloak-dodge#comments:id=198769
-- https://www.wowhead.com/item=33149/formula-enchant-cloak-stealth#comments:id=198769
-- https://www.wowhead.com/item=33152/formula-enchant-gloves-superior-agility#comments:id=198769
-- https://www.wowhead.com/item=33153/formula-enchant-gloves-threat#comments:id=198769
DELETE FROM npc_vendor WHERE item IN(33148,33149,33150,33151,33152,33153);
DELETE FROM npc_vendor_template WHERE item IN(33148,33149,33150,33151,33152,33153);

-- ========================
-- Quests & Reputation
-- ========================

-- Adjust rep reward for The Relic's Emanation in Blade's Edge Mountains
UPDATE quest_template set rewrepvalue1 = 150, rewmoneymaxlevel = 85900 WHERE entry = 11080;

-- Escape From Skettis Reward change
UPDATE quest_template SET rewmoneymaxlevel = 57000, Rewrepfaction1 = 1031, Rewrepvalue1 = 350, rewchoiceitemID1 = 28100 , rewchoiceitemID2 = 28101, rewchoiceitemcount1 = 2, rewchoiceitemcount2 = 2  WHERE entry = 11085;

-- Rep reward change for Talonsworn Forest-Rager
UPDATE creature_onkill_reputation SET rewonkillrepvalue1 = 15 WHERE Creature_id = 23029;

-- ========================
-- Raids
-- ========================

-- Tinhead - Immune to bleeds after patch 2.2
UPDATE creature_template SET MechanicImmuneMask=MechanicImmuneMask&~16384 WHERE entry IN(17547);

-- Patch 2.1 - Shade of Aran's Blizzard should now be more visible and its duration has been reduced
UPDATE `spell_template` SET `DurationIndex` = 9 WHERE `Id` = 29952; -- DBM 30secs

-- Patch ~ 2.2 - Spatial Distortion should stack up to 75% reduce
-- https://www.wowhead.com/spell=30007/spatial-distortion#comments:id=146819
-- https://www.wowhead.com/spell=29983/spatial-distortion#comments:id=68372
UPDATE `spell_template` SET `StackAmount` = 25 WHERE `Id` IN (29983,30008);

-- Patch 2.2 "Phoenix-Hawk Hatchlings now Wing Buffet less frequently."
UPDATE creature_ai_scripts SET event_param3=6000, event_param4=7500 WHERE id=2003801; -- 12000 15000

-- Patch 2.2 "Crimson Hand Centurions now deal less damage with Arcane Flurry."
UPDATE spell_template SET EffectBasePoints1=6524 WHERE id=37270; -- 5524

-- Patch 2.2 "Crimson Hand Battle Mages have had their Frost Attack damage reduced."
-- Frostbolt Volley
UPDATE spell_template SET EffectBasePoints1=3624 WHERE id=37262; -- 2124
-- Cone of Cold
UPDATE spell_template SET EffectBasePoints1=3687 WHERE id=37265; -- 2187
-- Blizzard
UPDATE spell_template SET EffectBasePoints1=3874 WHERE id=37263; -- 2374

-- http://wowwiki.wikia.com/wiki/Patch_2.2.0
-- The casting time of Archimonde's fear has been increased to 1.5 seconds, and he should now use his Fear ability on a much more consistent interval.
UPDATE spell_template SET CastingTimeIndex=4 WHERE id IN(31970); -- 1sec

-- Patch 2.2 - Archimonde's Soul Charge silence has been lowered in duration to 4 seconds - from 6 seconds http://wowwiki.wikia.com/wiki/Archimonde_(tactics)?oldid=824048
UPDATE `spell_template` SET `DurationIndex` = 32 WHERE `Id` = 32053;

-- Illidan
UPDATE spell_template SET EffectBasePoints2=1199 WHERE Id IN(40932); -- prenerf value for agonizing flames - Patch 2.1.2

-- Patch 2.2 - The health of Jaina and Thrall have been increased again from their temporarily lowered values. They should be able to absorb quite a bit more incidental damage during the event.
-- Lady Jaina Proudmoore (17772)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200' WHERE `entry` = '17772';
-- Thrall (17852)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200' WHERE `entry` = '17852';

-- ========================
-- Dungeons
-- ========================

-- Headless Horseman added, despawn the objects required to summon him in Scarlet Monastery
SET @CGUID := 1890000;
UPDATE gameobject SET spawnMask=0 WHERE guid BETWEEN @OGUID+200 AND @OGUID+212;

-- 2.1.2: Heroic Mode: Anzu, the Raven God may no longer be pulled far away from the location where he is summoned. 
UPDATE creature_template SET Leash=0 WHERE entry IN (23035);

-- Patch 2.2 - Decreases the maximum possible stacks of Temporus' Mortal Wound from 10 to 7.
UPDATE `spell_template` SET `StackAmount` = 10 WHERE `Id` = 31464;

-- Patch 2.2 - Increased recast time on Shadow Bolt Volley on Rift Keeper, also reduced damage on Heroic.
DELETE FROM `creature_ai_scripts` WHERE `id` IN (2110402,2110403);
INSERT INTO `creature_ai_scripts` (`id`,`creature_id`,`event_type`,`event_inverse_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action1_type`,`action1_param1`,`action1_param2`,`action1_param3`,`action2_type`,`action2_param1`,`action2_param2`,`action2_param3`,`action3_type`,`action3_param1`,`action3_param2`,`action3_param3`,`comment`) VALUES
('2110402','21104','0','0','100','1027','7200','12200','11900','25000','11','36275','0','0','0','0','0','0','0','0','0','0','Rift Keeper (Normal) - Cast Shadow Bolt Volley'),
('2110403','21104','0','0','100','1029','7200','12000','11100','20000','11','38533','0','0','0','0','0','0','0','0','0','0','Rift Keeper (Heroic) - Cast Shadow Bolt Volley');

-- Patch 2.2 - Thorgrin the Tender's Enrage ability now increases his melee damage by 75% instead of 110%.
UPDATE `spell_template` SET `EffectBasePoints1` = 109 WHERE `Id` = 34670; -- 74

-- Dragonbreath Chili - 1 SP coeff before 2.2
DELETE FROM spell_bonus_data WHERE entry IN(15851);
INSERT INTO spell_bonus_data(entry,direct_bonus) VALUES
(15851,1);

-- =============
-- Custom Change
-- =============
-- More items that were added in 2.2 but we missed on launch. Don't allow crafting
UPDATE spell_template SET ReagentCount1=999999 WHERE Effect1=24 AND EffectItemType1 IN(31860,31862,31864,31866,31869);

