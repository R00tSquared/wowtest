/* DBScriptData
DBName: Coilfang Reservoir - Serpentshrine Cavern
DBScriptName: instance_serpent_shrine
DB%Complete:
DBComment: PRE-NERF
EndDBScriptData */
/*
SET @CGUID := 5480000; -- creatures
SET @OGUID := 5480000; -- gameobjects
SET @PGUID := 48700; -- pools

-- =========
-- CREATURES
-- =========

INSERT INTO `creature_movement` (`id`, `point`, `PositionX`, `PositionY`, `PositionZ`, `orientation`, `waittime`, `ScriptId`) VALUES
-- Underbog Colossus bottom of elevators
-- (@CGUID+900, 1, 19.619, -56.7943, -72.0772, 100, 0, 0),
-- (@CGUID+900, 2, 29.2468, -52.0036, -72.4646, 100, 0, 0),
-- (@CGUID+900, 3, 19.619, -56.7943, -72.0772, 100, 0, 0),
-- (@CGUID+900, 4, -11.0251, -66.2496, -71.2579, 100, 0, 0),
-- (@CGUID+900, 5, -37.172, -64.7938, -69.9687, 100, 0, 0),
-- (@CGUID+900, 6, -11.0251, -66.2496, -71.2579, 100, 0, 0),
-- Serpentshrine Lurker patrol in hallway to Leo
(@CGUID+906, 1, 353.608, -300.843, 17.628, 100, 0, 0),
(@CGUID+906, 2, 353.445, -291.711, 18.635, 100, 0, 0),
(@CGUID+906, 3, 344.068, -275.768, 19.354, 100, 0, 0),
(@CGUID+906, 4, 330.335, -265.038, 16.553, 100, 0, 0),
(@CGUID+906, 5, 306.721, -254.353, 12.242, 100, 0, 0),
(@CGUID+906, 6, 272.244, -265.224, 3.426, 100, 0, 0),
(@CGUID+906, 7, 307.021, -254.671, 12.281, 100, 0, 0),
(@CGUID+906, 8, 330.072, -265.019, 16.507, 100, 0, 0),
(@CGUID+906, 9, 342.371, -273.688, 19.474, 100, 0, 0),
(@CGUID+906, 10, 353.713,-291.472, 18.671, 100, 0, 0),
-- Serpentshrine Lurker patrol circling central pillar in Leo room
(@CGUID+908, 1, 372.905, -326.547, 19.362, 100, 0, 0),
(@CGUID+908, 2, 380.844, -340.208, 21.649, 100, 0, 0),
(@CGUID+908, 3, 378.401, -357.754, 22.473, 100, 0, 0),
(@CGUID+908, 4, 366.598, -368.446, 22.027, 100, 0, 0),
(@CGUID+908, 5, 334.533, -371.777, 22.225, 100, 0, 0),
(@CGUID+908, 6, 323.227, -366.982, 21.989, 100, 0, 0),
(@CGUID+908, 7, 318.575, -348.664, 21.796, 100, 0, 0),
(@CGUID+908, 8, 321.625, -332.176, 19.762, 100, 0, 0),
(@CGUID+908, 9, 336.084, -318.949, 18.575, 100, 0, 0),
(@CGUID+908, 10, 320.359, -332.583, 19.864, 100, 0, 0),
(@CGUID+908, 11, 317.804, -350.254, 21.863, 100, 0, 0),
(@CGUID+908, 12, 323.757, -368.161, 22.046, 100, 0, 0),
(@CGUID+908, 13, 335.763, -373.027, 22.287, 100, 0, 0),
(@CGUID+908, 14, 370.481, -368.744, 22.093, 100, 0, 0),
(@CGUID+908, 15, 378.484, -356.029, 22.432, 100, 0, 0),
(@CGUID+908, 16, 380.844, -340.208, 21.649, 100, 0, 0),
-- extra Underbog Colossus on ramp from Lurker platforms to Leo/Tidewalker
(@CGUID+920, 1, 189.004, -367.237, 11.98, 100, 0, 0),
(@CGUID+920, 2, 193.224, -378.813, 10.5444, 100, 0, 0),
(@CGUID+920, 3, 208.21, -393.331, 8.41086, 100, 0, 0),
(@CGUID+920, 4, 218.647, -397.121, 5.21746, 100, 0, 0),
(@CGUID+920, 7, 225.968, -402.185, 1.95111, 100, 0, 0),
(@CGUID+920, 8, 229.764, -410.266, -1.67851, 100, 0, 0),
(@CGUID+920, 9, 228.532, -416.856, -4.43269, 100, 0, 0),
(@CGUID+920, 10, 214.638, -424.839, -4.43271, 100, 0, 0),
(@CGUID+920, 11, 201.527, -433.372, -4.43271, 100, 0, 0),
(@CGUID+920, 12, 176.575, -435.087, -1.4202, 100, 0, 0),
(@CGUID+920, 13, 157.968, -434.233, 0.220847, 100, 0, 0),
(@CGUID+920, 14, 176.595, -437.761, -1.44509, 100, 0, 0),
(@CGUID+920, 15, 201.867, -434.319, -4.4327, 100, 0, 0),
(@CGUID+920, 16, 228.684, -417.839, -4.43269, 100, 0, 0),
(@CGUID+920, 17, 230.065, -410.237, -1.5854, 100, 0, 0),
(@CGUID+920, 18, 226.24, -402.402, 1.89719, 100, 0, 0),
(@CGUID+920, 19, 219.086, -397.407, 5.09838, 100, 0, 0),
(@CGUID+920, 20, 208.599, -393.628, 8.31259, 100, 0, 0),
(@CGUID+920, 21, 193.416, -379.063, 10.4503, 100, 0, 0),
(@CGUID+920, 22, 189.537, -367.228, 11.949, 100, 0, 0);
-- this Underbog Colossus doesn't need to walk as far when there is an extra Underbog Colossus in pre-nerf mode
DELETE FROM creature_movement WHERE id=@CGUID+147;
INSERT INTO `creature_movement` (`id`, `point`, `PositionX`, `PositionY`, `PositionZ`, `orientation`, `waittime`, `ScriptId`) VALUES
(@CGUID+147, 1, 189.166, -355.996, 12.6764, 1.57066, 0, 0),
(@CGUID+147, 2, 198.22, -335.836, 13.7258, 1.13476, 0, 0),
(@CGUID+147, 3, 209.409, -319.075, 10.6176, 0.981607, 0, 0),
(@CGUID+147, 4, 212.819, -299.114, 4.41499, 100, 0, 0),
(@CGUID+147, 5, 219.637, -284.966, -2.02757, 100, 0, 0),
(@CGUID+147, 6, 237.06, -259.621, -1.99051, 100, 1000, 0),
(@CGUID+147, 7, 219.374, -284.112, -2.02644, 100, 0, 0),
(@CGUID+147, 8, 213.461, -299.802, 4.52623, 100, 0, 0),
(@CGUID+147, 9, 209.256, -318.906, 10.5753, 100, 0, 0);

INSERT INTO `creature_addon` (`guid`, `mount`, `bytes1`, `b2_0_sheath`, `b2_1_flags`, `emote`, `moveflags`, `auras`) VALUES
(@CGUID+912, 0, 0, 0, 0, 173, 0, NULL), -- Greyheart Skulker
(@CGUID+919, 0, 0, 0, 0, 173, 0, NULL); -- Greyheart Skulker

-- REPLACE INTO `creature_template_addon` (`entry`, `mount`, `bytes1`, `b2_0_sheath`, `b2_1_flags`, `emote`, `moveflags`, `auras`) VALUES

INSERT INTO `creature_linking` (`guid`, `master_guid`, `flag`) VALUES
-- bottom of elevators
-- (@CGUID+900, @CGUID+41, 1024), -- Underbog Giant -> Hydross the Unstable (Lady Vashj)

-- (@CGUID+902, @CGUID+901, 1167), -- Coilfang Hate-Screamer -> Coilfang Beast-Tamer
-- (@CGUID+903, @CGUID+901, 1167), -- Coilfang Hate-Screamer -> Coilfang Beast-Tamer
-- (@CGUID+901, @CGUID+41, 1024), -- Coilfang Beast-Tamer -> Hydross the Unstable (Lady Vashj)

-- extras in group guarding hallway entrance to Leo
(@CGUID+904, @CGUID+186, 1167), -- Coilfang Serpentguard -> Coilfang Serpentguard
(@CGUID+905, @CGUID+186, 1167), -- Coilfang Serpentguard -> Coilfang Serpentguard

-- patrol in hallway to Leo
(@CGUID+906, @CGUID+40, 1024), -- Serpentshrine Lurker -> Leotheras the Blind
(@CGUID+907, @CGUID+906, 1679), -- Serpentshrine Lurker -> Serpentshrine Lurker

-- patrol around central pillar in Leo room
(@CGUID+908, @CGUID+40, 1024), -- Serpentshrine Lurker -> Leotheras the Blind
(@CGUID+909, @CGUID+908, 1679), -- Serpentshrine Lurker -> Serpentshrine Lurker

-- extras in group guarding hallway entrance to Tidewalker
(@CGUID+910, @CGUID+189, 1167), -- Coilfang Serpentguard -> Coilfang Serpentguard
(@CGUID+911, @CGUID+189, 1167), -- Coilfang Serpentguard -> Coilfang Serpentguard

-- left side entering Leo room
(@CGUID+912, @CGUID+40, 1024), -- Greyheart Skulker -> Leotheras the Blind
(@CGUID+913, @CGUID+912, 1167), -- Greyheart Nether-Mage -> Greyheart Skulker
(@CGUID+914, @CGUID+912, 1167), -- Greyheart Nether-Mage -> Greyheart Skulker

-- right side entering Leo room
(@CGUID+915, @CGUID+917, 1167),
(@CGUID+916, @CGUID+917, 1167),
(@CGUID+917, @CGUID+40, 1024), -- Serpentshrine Lurker -> Leotheras the Blind
(@CGUID+918, @CGUID+917, 1167),
(@CGUID+919, @CGUID+917, 1167),

-- extra Underbog Colossus on ramp from Lurker platforms to Leo/Tidewalker
(@CGUID+920, @CGUID+154, 1024); -- Underbog Colossus -> World Trigger (Not Immune PC)

-- REPLACE INTO `creature_linking_template` (`entry`, `map`, `master_entry`, `flag`, `search_range`) VALUES

INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecsmin`, `spawntimesecsmax`, `spawndist`, `currentwaypoint`, `DeathState`, `MovementType`) VALUES
-- bottom of elevators (unconfirmed)
-- (@CGUID+900, 21251, 548, 1, -8.32471, -65.7848, -71.2575, 0.255056, 7200, 7200, 0, 0, 0, 2), -- Underbog Giant
-- (@CGUID+901, 21221, 548, 1, -59.1932, -65.7469, -68.6451, 0.185156, 7200, 7200, 0, 0, 0, 0), -- Coilfang Beast-Tamer
-- (@CGUID+902, 21339, 548, 1, -61.9279, -60.3046, -68.7302, 0.178873, 7200, 7200, 0, 0, 0, 0), -- Coilfang Hate-Screamer
-- (@CGUID+903, 21339, 548, 1, -58.5528, -71.4702, -68.6238, 0.166304, 7200, 7200, 0, 0, 0, 0), -- Coilfang Hate-Screamer

-- entrance to Leo hallway
(@CGUID+904, 21298, 548, 3, 257.835, -268.0377, 0.47407, 3.292999, 7200, 7200, 0, 0, 0, 0), -- Coilfang Serpentguard
(@CGUID+905, 21298, 548, 3, 257.601, -262.4586, -0.04396, 3.265509, 7200, 7200, 0, 0, 0, 0), -- Coilfang Serpentguard

-- patroling hallway to Leo
(@CGUID+906, 21863, 548, 3, 354.681091, -309.904327, 18.099518, 1.618499, 7200, 7200, 0, 0, 0, 2), -- Serpentshrine Lurker
(@CGUID+907, 21863, 548, 3, 354.875305, -313.280212, 18.276270, 1.618500, 7200, 7200, 0, 0, 0, 0), -- Serpentshrine Lurker

-- circling the central pillar in Leo's room
(@CGUID+908, 21863, 548, 3, 378.640839, -340.682312, 21.587902, 1.767724, 7200, 7200, 0, 0, 0, 2), -- Serpentshrine Lurker
(@CGUID+909, 21863, 548, 3, 381.453003, -339.622284, 21.626650, 1.936585, 7200, 7200, 0, 0, 0, 0), -- Serpentshrine Lurker

-- entrance to Tidewalker hallway
(@CGUID+910, 21298, 548, 3, 252.548096, -665.591064, -7.363951, 2.427455, 7200, 7200, 0, 0, 0, 0), -- Coilfang Serpentguard
(@CGUID+911, 21298, 548, 3, 243.559921, -671.528809, -7.363951, 2.396039, 7200, 7200, 0, 0, 0, 0), -- Coilfang Serpentguard

-- left side, entering Leo room
(@CGUID+912, 21232, 548, 3, 377.94885, -320.996704, 19.575535, 1.910665, 7200, 7200, 0, 0, 0, 0), -- Greyheart Skulker
(@CGUID+913, 21230, 548, 3, 368.756592, -308.981598, 17.937666, 4.109779, 7200, 7200, 3, 0, 0, 1), -- Greyheart Nether-Mage
(@CGUID+914, 21230, 548, 3, 372.588989, -314.339172, 18.774731, 3.925212, 7200, 7200, 3, 0, 0, 1), -- Greyheart Nether-Mage

-- right side, entering Leo room
(@CGUID+915, 21230, 548, 3, 320.349182, -316.600769, 18.494764, 5.448894, 7200, 7200, 3, 0, 0, 1), -- Greyheart Nether-Mage
(@CGUID+916, 21230, 548, 3, 311.971588, -325.756104, 19.306152, 5.896566, 7200, 7200, 0, 0, 0, 0), -- Greyheart Nether-Mage
(@CGUID+917, 21863, 548, 3, 304.889343, -331.012360, 20.325111, 5.747330, 7200, 7200, 0, 0, 0, 0), -- Serpentshrine Lurker
(@CGUID+918, 21229, 548, 3, 318.225708, -323.089996, 18.954302, 5.971169, 7200, 7200, 0, 0, 0, 0), -- Greyheart Tidecaller
(@CGUID+919, 21232, 548, 3, 314.107910, -313.317444, 18.374218, 3.214422, 7200, 7200, 0, 0, 0, 0), -- Greyheart Skulker

-- extra Underbog Colossus on ramp from Lurker platforms to Leo/Tidewalker
(@CGUID+920, 21251, 548, 3, 186.209503, -361.883301, 12.45, 5.046, 7200, 7200, 0, 0, 0, 2); -- Underbog Colossus

-- Greyheart Skulker - 1 dagger and 1 hammer
UPDATE creature SET equipment_id=2123201 WHERE guid IN (@CGUID+912,@CGUID+919);

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
-- INSERT INTO `dbscripts_on_creature_death` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_relay` (`id`, `delay`, `command`, `datalong`, `datalong2`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscript_random_templates` (id, type, target_id, chance) VALUES
-- INSERT INTO `dbscripts_on_go_use` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_event` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_spell` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_gossip` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_quest_start` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_quest_end` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
*/

