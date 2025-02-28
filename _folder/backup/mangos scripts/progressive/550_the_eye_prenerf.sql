/* DBScriptData
DBName: Tempest Keep - The Eye
DBScriptName: instance_the_eye
DB%Complete:
DBComment: PRE-NERF
EndDBScriptData */
/*
SET @CGUID := 5500000; -- creatures
SET @OGUID := 5500000; -- gameobjects
SET @PGUID := 48800; -- pools

-- =========
-- CREATURES
-- =========

INSERT INTO `creature_movement` (`id`, `point`, `PositionX`, `PositionY`, `PositionZ`, `orientation`, `waittime`, `ScriptId`) VALUES
(@CGUID+990, 1, 647.52, -85.53, 46.8, 100, 0, 0),
(@CGUID+990, 2, 619.04, -111.81, 46.3, 100, 2000, 0),
(@CGUID+990, 3, 647.52, -85.53, 46.8, 100, 0, 0),
(@CGUID+990, 4, 703.24, -7.29, 46.78, 100, 1500, 0),
(@CGUID+993, 1, 647.53, 84.78, 46.8, 100, 0, 0),
(@CGUID+993, 2, 619.86, 109.23, 46.3, 100, 1700, 0),
(@CGUID+993, 3, 647.53, 84.78, 46.8, 100, 0, 0),
(@CGUID+993, 4, 701.77, 6.18, 46.78, 100, 1700, 0),
(@CGUID+996, 1, 740.524, -0.7594, 46.7795, 100, 0, 0),
(@CGUID+996, 2, 707.74, -0.7159, 46.7795, 100, 10000, 0),
(@CGUID+996, 3, 709.564, -0.6971, 46.7795, 100, 0, 0),
(@CGUID+996, 4, 742.029, -0.7949, 46.7796, 100, 10000, 0);

-- INSERT INTO `creature_movement_template` (`entry`, `pathId`, `point`, `PositionX`, `PositionY`, `PositionZ`, `orientation`, `waittime`, `ScriptId`) VALUES
-- INSERT INTO `creature_addon` (`guid`, `mount`, `bytes1`, `b2_0_sheath`, `b2_1_flags`, `emote`, `moveflags`, `auras`) VALUES
-- REPLACE INTO `creature_template_addon` (`entry`, `mount`, `bytes1`, `b2_0_sheath`, `b2_1_flags`, `emote`, `moveflags`, `auras`) VALUES

INSERT INTO `creature_linking` (`guid`, `master_guid`, `flag`) VALUES
(@CGUID+990, @CGUID+5, 1024), -- Crimson Hand Inquisitor -> Kael'thas Sunstrider
(@CGUID+993, @CGUID+5, 1024), -- Crimson Hand Inquisitor -> Kael'thas Sunstrider
(@CGUID+996, @CGUID+5, 1024), -- Crimson Hand Inquisitor -> Kael'thas Sunstrider

(@CGUID+991, @CGUID+990, 1679), -- Crimson Hand Centurion -> Crimson Hand Inquisitor
(@CGUID+992, @CGUID+990, 1679), -- Crimson Hand Centurion -> Crimson Hand Inquisitor

(@CGUID+994, @CGUID+993, 1679), -- Crimson Hand Centurion -> Crimson Hand Inquisitor
(@CGUID+995, @CGUID+993, 1679), -- Crimson Hand Centurion -> Crimson Hand Inquisitor

(@CGUID+997, @CGUID+996, 1679), -- Crimson Hand Centurion -> Crimson Hand Inquisitor
(@CGUID+998, @CGUID+996, 1679); -- Crimson Hand Centurion -> Crimson Hand Inquisitor

-- REPLACE INTO `creature_linking_template` (`entry`, `map`, `master_entry`, `flag`, `search_range`) VALUES

INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecsmin`, `spawntimesecsmax`, `spawndist`, `currentwaypoint`, `DeathState`, `MovementType`) VALUES
(@CGUID+990, 20050, 550, 1, 703.325, -7.25872, 46.78, 4.11, 7200, 7200, 0, 0, 0, 2),
(@CGUID+991, 20048, 550, 1, 703.076, -3.35933, 46.7789, 4.08011, 7200, 7200, 0, 0, 0, 0),
(@CGUID+992, 20048, 550, 1, 706.623, -5.87168, 46.7789, 4.04229, 7200, 7200, 0, 0, 0, 0),

(@CGUID+993, 20050, 550, 1, 699.054, 10.1159, 46.781, 5.31642, 7200, 7200, 0, 0, 0, 2),
(@CGUID+994, 20048, 550, 1, 704.941, 5.2947, 46.7798, 2.11744, 7200, 7200, 0, 0, 0, 0),
(@CGUID+995, 20048, 550, 1, 701.758, 3.09123, 46.7798, 2.11744, 7200, 7200, 0, 0, 0, 0),

(@CGUID+996, 20050, 550, 1, 742.029, -0.7949, 46.7796, 3.1573, 7200, 7200, 0, 0, 0, 2),
(@CGUID+997, 20048, 550, 1, 741.94, 4.8253, 46.7796, 3.1573, 7200, 7200, 0, 0, 0, 0),
(@CGUID+998, 20048, 550, 1, 742.114, -6.2403, 46.7788, 3.1573, 7200, 7200, 0, 0, 0, 0);

-- =======
-- POOLING
-- =======

-- INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`) VALUES
-- INSERT INTO `pool_template` (`entry`, `max_limit`, `description`) VALUES
-- INSERT INTO `pool_creature` (`guid`, `pool_entry`, `chance`, `description`) VALUES
-- INSERT INTO `pool_creature_template` (`id`, `pool_entry`, `chance`, `description`) VALUES

-- =========
-- DBSCRIPTS
-- =========

-- INSERT INTO `dbscripts_on_creature_movement` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_relay` (`id`, `delay`, `command`, `datalong`, `datalong2`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscript_random_templates` (`id`, `type`, `target_id`, `chance`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_creature_death` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_go_use` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_go_template_use` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_event` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_spell` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_gossip` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_quest_start` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_quest_end` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES

*/
