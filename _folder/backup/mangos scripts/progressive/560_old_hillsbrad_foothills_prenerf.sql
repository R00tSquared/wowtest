/* DBScriptData
DBName: Caverns of Time - Old Hillsbrad Foothills
DBScriptName: instance_old_hillsbrad
DB%Complete: 80
DBComment:

Patch 2.0.7 - Durnholde Lookouts will now properly despawn after all 5 of the Baracks in Caverns of Time: Escape from Durnholde are burned down.
-- Linking them to Durnholde Rifleman infront of last Hut to be burned

Patch 2.4.0 - Durnholde Lookouts no longer spawn in the Heroic version of the instance. The placement of creatures around Durnholde Keep in Heroic mode is now identical to that of Normal mode.

There are 3 patrols of these, they roam with dogs (which see through stealth).
They respawn after death in the same three places
The respawn is between 5-10 minutes.
- at the top of the pit area with the on lookers
- at the bottom of the pit near its entrance
- at the front right corner of the prison house furthest in the back

The Durnholde Lookouts are the main obstacle in heroic old hillsbrad, at least inside Durnholde keep.

They will spawn 2x lvl72 elites that are non-ccable, non-blindable, immune to fear, charm and sleep. Hit for 2-5k depending on armor type, and have hp in the 48-55k range.
When you aggro a lookout they will shout for reinforcements, and the adds wiill spawn, after your group wipes they will despawn.
They are immune to MC. No CC works. Immune to Sheep, Seduce, Hammer of Justice, even stuns and fear.

The way we dealt with this is CC'ing the lookout first using seduce, killing the dog, then killing the lookout. After the initial CC they don't do the shout.
Sheep should work as good, sap is out of the question due to the dogs.
EndDBScriptData */

SET @CGUID := 5600000; -- creatures
SET @OGUID := 5600000; -- gameobjects
SET @PGUID := 49600; -- pools

-- =========
-- CREATURES
-- =========

INSERT INTO `creature_movement` (`id`, `point`, `PositionX`, `PositionY`, `PositionZ`, `orientation`, `waittime`, `ScriptId`) VALUES
(@CGUID+901, 1, 2194.76, 228.093, 53.2216, 0, 0, 0),
(@CGUID+901, 2, 2177.35, 222.908, 52.5617, 0, 0, 0),
(@CGUID+901, 3, 2155.81, 211.125, 52.9491, 0, 0, 0),
(@CGUID+901, 4, 2136.14, 190.397, 52.6035, 0, 0, 0),
(@CGUID+901, 5, 2123.86, 177.449, 52.5751, 0, 0, 0),
(@CGUID+901, 6, 2118.74, 164.827, 52.473, 0, 0, 0),
(@CGUID+901, 7, 2111.26, 150.577, 52.441, 0, 0, 0),
(@CGUID+901, 8, 2109.52, 133.664, 52.441, 0, 0, 0),
(@CGUID+901, 9, 2112.11, 113.823, 52.5069, 0, 0, 0),
(@CGUID+901, 10, 2116.1, 88.3774, 52.5566, 0, 0, 0),
(@CGUID+901, 11, 2119.94, 64.779, 52.668, 0, 0, 0),
(@CGUID+901, 12, 2126.07, 55.7297, 52.4422, 0, 0, 0),
(@CGUID+901, 13, 2142.26, 44.6962, 52.5726, 0, 0, 0),
(@CGUID+901, 14, 2126.15, 55.8954, 52.441, 0, 0, 0),
(@CGUID+901, 15, 2119.89, 64.841, 52.6693, 0, 0, 0),
(@CGUID+901, 16, 2119.19, 77.1831, 52.6315, 0, 0, 0),
(@CGUID+901, 17, 2114.45, 101.229, 52.4411, 0, 0, 0),
(@CGUID+901, 18, 2110.13, 129.652, 52.4412, 0, 0, 0),
(@CGUID+901, 19, 2109.8, 142.596, 52.4415, 0, 0, 0),
(@CGUID+901, 20, 2111.71, 151, 52.4422, 0, 0, 0),
(@CGUID+901, 21, 2118.62, 163.934, 52.4958, 0, 0, 0),
(@CGUID+901, 22, 2124.02, 177.071, 52.6379, 0, 0, 0),
(@CGUID+901, 23, 2128.47, 184.045, 52.8829, 0, 0, 0),
(@CGUID+901, 24, 2141.11, 193.758, 52.4788, 0, 0, 0),
(@CGUID+901, 25, 2155.71, 210.856, 52.963, 0, 0, 0),
(@CGUID+901, 26, 2166.12, 216.871, 52.784, 0, 0, 0),

(@CGUID+902, 1, 2179.58, 272.553, 53.7907, 0, 0, 0),
(@CGUID+902, 2, 2174.51, 273.75, 54.7804, 0, 0, 0),
(@CGUID+902, 3, 2167.73, 270.116, 53.8525, 0, 0, 0),
(@CGUID+902, 4, 2158.07, 262.467, 54.2476, 0, 0, 0),
(@CGUID+902, 5, 2148.58, 254.01, 53.6444, 0, 0, 0),
(@CGUID+902, 6, 2142.06, 248.19, 54.5281, 0, 0, 0),
(@CGUID+902, 7, 2141.05, 241.971, 54.0081, 0, 0, 0),
(@CGUID+902, 8, 2151.24, 228.01, 52.5662, 0, 0, 0),
(@CGUID+902, 9, 2157.59, 225.422, 52.5796, 0, 0, 0),
(@CGUID+902, 10, 2163.86, 228.987, 52.4411, 0, 0, 0),
(@CGUID+902, 11, 2171.11, 238.58, 52.4803, 0, 0, 0),
(@CGUID+902, 12, 2186.56, 250.708, 52.672, 0, 0, 0),
(@CGUID+902, 13, 2195.97, 250.156, 52.441, 0, 0, 0),
(@CGUID+902, 14, 2204.23, 245.158, 53.3357, 0, 0, 0),
(@CGUID+902, 15, 2220.32, 235.855, 52.5448, 0, 0, 0),
(@CGUID+902, 16, 2225.11, 237.303, 53.2101, 0, 0, 0),
(@CGUID+902, 17, 2231.1, 247.145, 53.7591, 0, 0, 0),
(@CGUID+902, 18, 2233.59, 253.688, 54.8861, 0, 0, 0),
(@CGUID+902, 19, 2233.58, 256.145, 55.5751, 0, 0, 0),
(@CGUID+902, 20, 2228.66, 260.389, 54.1031, 0, 0, 0),
(@CGUID+902, 21, 2226.51, 259.921, 53.4273, 0, 0, 0),
(@CGUID+902, 22, 2222.34, 253.34, 53.7322, 0, 0, 0),
(@CGUID+902, 23, 2206.44, 263.036, 54.0515, 0, 0, 0),
(@CGUID+902, 24, 2196.3, 260.947, 54.0427, 0, 0, 0),
(@CGUID+902, 25, 2194.69, 256.858, 54.0644, 0, 0, 0),
(@CGUID+902, 26, 2191.63, 251.755, 52.4522, 0, 0, 0),
(@CGUID+902, 27, 2189.27, 255.95, 52.4471, 0, 0, 0),
(@CGUID+902, 28, 2184.46, 262.837, 52.4412, 0, 0, 0),
(@CGUID+902, 29, 2187.25, 272.681, 52.8813, 0, 0, 0),

(@CGUID+903, 1, 2133.25, 175.897, 68.2518, 0, 0, 0),
(@CGUID+903, 2, 2142.45, 168.965, 66.2217, 0, 0, 0),
(@CGUID+903, 3, 2143.95, 162.492, 65.283, 0, 0, 0),
(@CGUID+903, 4, 2139.32, 154.392, 67.4044, 0, 0, 0),
(@CGUID+903, 5, 2138.23, 143.395, 70.7358, 0, 0, 0),
(@CGUID+903, 6, 2139.97, 132.573, 74.0745, 0, 0, 0),
(@CGUID+903, 7, 2142.59, 126.72, 75.6225, 0, 0, 0),
(@CGUID+903, 8, 2146.19, 125.347, 76.3226, 0, 0, 0),
(@CGUID+903, 9, 2153.81, 128.848, 79.1896, 0, 0, 0),
(@CGUID+903, 10, 2161.36, 135.221, 83.6257, 0, 0, 0),
(@CGUID+903, 11, 2169.32, 144.168, 87.0686, 0, 0, 0),
(@CGUID+903, 12, 2176.57, 157.486, 87.5667, 0, 0, 0),
(@CGUID+903, 13, 2182.25, 166.023, 88.0925, 0, 0, 0),
(@CGUID+903, 14, 2190.15, 171.974, 88.9953, 0, 0, 0),
(@CGUID+903, 15, 2198.18, 175.07, 90.0099, 0, 0, 0),
(@CGUID+903, 16, 2211.11, 177.332, 93.1255, 0, 0, 0),
(@CGUID+903, 17, 2218.76, 179.856, 96.8824, 0, 0, 0),
(@CGUID+903, 18, 2211.71, 177.317, 93.3128, 0, 0, 0),
(@CGUID+903, 19, 2198.41, 175.057, 90.0273, 0, 0, 0),
(@CGUID+903, 20, 2190.48, 171.961, 89.0125, 0, 0, 0),
(@CGUID+903, 21, 2182.36, 165.732, 88.0739, 0, 0, 0),
(@CGUID+903, 22, 2176.82, 157.265, 87.5719, 0, 0, 0),
(@CGUID+903, 23, 2169.84, 144.243, 87.1995, 0, 0, 0),
(@CGUID+903, 24, 2161.79, 135.187, 83.7742, 0, 0, 0),
(@CGUID+903, 25, 2153.85, 128.339, 79.1086, 0, 0, 0),
(@CGUID+903, 26, 2146.26, 125.303, 76.3378, 0, 0, 0),
(@CGUID+903, 27, 2142.64, 126.864, 75.6126, 0, 0, 0),
(@CGUID+903, 28, 2139.95, 133.04, 73.9752, 0, 0, 0),
(@CGUID+903, 29, 2138.51, 143.182, 70.7858, 0, 0, 0),
(@CGUID+903, 30, 2139.87, 154.51, 67.3187, 0, 0, 0),
(@CGUID+903, 31, 2144.53, 162.434, 65.2721, 0, 0, 0),
(@CGUID+903, 32, 2142.94, 168.955, 66.2216, 0, 0, 0),
(@CGUID+903, 33, 2133.65, 175.448, 68.0162, 0, 0, 0),
(@CGUID+903, 34, 2121.96, 184.319, 69.1957, 0, 0, 0),
(@CGUID+903, 35, 2111.52, 191.525, 66.2216, 0, 0, 0),
(@CGUID+903, 36, 2105.32, 194.886, 65.1854, 0, 0, 0),
(@CGUID+903, 37, 2097.08, 196.945, 65.2138, 0, 0, 0),
(@CGUID+903, 38, 2087.51, 206.91, 64.8803, 0, 0, 0),
(@CGUID+903, 39, 2073.1, 218.576, 64.8744, 0, 0, 0),
(@CGUID+903, 40, 2087.7, 206.877, 64.8792, 0, 0, 0),
(@CGUID+903, 41, 2097.34, 196.53, 65.2196, 0, 0, 0),
(@CGUID+903, 42, 2106.11, 194.526, 65.4583, 0, 0, 0),
(@CGUID+903, 43, 2111.96, 191.221, 66.2215, 0, 0, 0),
(@CGUID+903, 44, 2119.36, 186.393, 68.9396, 0, 0, 0);

-- INSERT INTO `creature_movement_template` (`entry`, `pathId`, `point`, `PositionX`, `PositionY`, `PositionZ`, `orientation`, `waittime`, `ScriptId`) VALUES

-- INSERT INTO `creature_addon` (`guid`, `mount`, `bytes1`, `b2_0_sheath`, `b2_1_flags`, `emote`, `moveflags`, `auras`) VALUES

-- REPLACE INTO `creature_template_addon` (`entry`, `mount`, `bytes1`, `b2_0_sheath`, `b2_1_flags`, `emote`, `moveflags`, `auras`) VALUES

INSERT INTO `creature_linking` (`guid`, `master_guid`, `flag`) VALUES
(@CGUID+901, @CGUID+395, 1024), -- Durnholde Lookout -> Durnholde Rifleman infront of last Hut
(@CGUID+902, @CGUID+395, 1024), -- Durnholde Lookout -> Durnholde Rifleman
(@CGUID+903, @CGUID+395, 1024), -- Durnholde Lookout -> Durnholde Rifleman
(@CGUID+911, @CGUID+901, 1667), -- Durnholde Tracking Hound -> Durnholde Lookout
(@CGUID+912, @CGUID+902, 1667), -- Durnholde Tracking Hound -> Durnholde Lookout
(@CGUID+913, @CGUID+903, 1667); -- Durnholde Tracking Hound -> Durnholde Lookout

-- REPLACE INTO `creature_linking_template` (`entry`, `map`, `master_entry`, `flag`, `search_range`) VALUES

-- Patch 2.0.7 - Durnholde Lookouts will now properly despawn after all 5 of the Baracks in Caverns of Time: Escape from Durnholde are burned down.
-- Not sure how we will implement this

-- Patch 2.4.0 - Durnholde Lookouts no longer spawn in the Heroic version of the instance. The placement of creatures around Durnholde Keep in Heroic mode is now identical to that of Normal mode.
UPDATE `creature` SET `spawnMask` = 1 WHERE `guid` IN (@CGUID+375,@CGUID+379,@CGUID+387); -- Durnholde Rifleman only on normal, Durnholde Rifleman
UPDATE `creature` SET `spawnMask` = 1 WHERE `guid` IN (@CGUID+419,@CGUID+423,@CGUID+424); -- Durnholde Tracking Hound normal as guid is primary key for creature_linking
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecsmin`, `spawntimesecsmax`, `spawndist`, `currentwaypoint`, `DeathState`, `MovementType`) VALUES
(@CGUID+901, 22128, 560, 2, 2166.98, 217.204, 52.8188, 0.32192, 300, 600, 0, 0, 0, 2), -- Durnholde Lookout CGUID+375
(@CGUID+902, 22128, 560, 2, 2186.06, 272.098, 52.7956, 3.30242, 300, 600, 0, 0, 0, 2), -- Durnholde Lookout @CGUID+379
(@CGUID+903, 22128, 560, 2, 2124.23, 182.621, 69.4139, 5.70349, 300, 600, 0, 0, 0, 2), -- Durnholde Lookout @CGUID+387
(@CGUID+911, 17840, 560, 2, 2164.4, 216.06, 52.6709, 0.424021, 7200, 7200, 0, 0, 0, 0), -- Durnholde Tracking Hound @CGUID+423
(@CGUID+912, 17840, 560, 2, 2187.73, 272.262, 52.7267, 3.14534, 7200, 7200, 0, 0, 0, 0), -- Durnholde Tracking Hound @CGUID+424
(@CGUID+913, 17840, 560, 2, 2122.54, 183.843, 69.253, 5.73098, 7200, 7200, 0, 0, 0, 0); -- Durnholde Tracking Hound @CGUID+419

-- ===========
-- GAMEOBJECTS
-- ===========

-- INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`) VALUES

-- ======
-- EVENTS
-- ======

-- INSERT INTO `game_event_creature` (`guid`, `event`) VALUES
-- INSERT INTO `game_event_creature_data` (`guid`, `entry_id`, `modelid`, `equipment_id`, `spell_start`, `spell_end`, `event`) VALUES
-- INSERT INTO `game_event_gameobject` (`guid`, `event`) VALUES

-- =======
-- POOLING
-- =======

-- INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`) VALUES

-- INSERT INTO `pool_template` (`entry`, `max_limit`, `description`) VALUES

-- INSERT INTO `pool_creature` (`guid`, `pool_entry`, `chance`, `description`) VALUES

-- INSERT INTO `pool_creature_template` (`id`, `pool_entry`, `chance`, `description`) VALUES

-- INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`) VALUES

-- INSERT INTO `pool_gameobject_template` (`id`, `pool_entry`, `chance`, `description`) VALUES

-- =========
-- DBSCRIPTS
-- =========

-- INSERT INTO `dbscripts_on_creature_movement` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_creature_death` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_go_use` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_go_template_use` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_relay` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_event` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_spell` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_gossip` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_quest_start` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscripts_on_quest_end` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `buddy_entry`, `search_radius`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `comments`) VALUES
-- INSERT INTO `dbscript_random_templates` (`id`, `type`, `target_id`, `chance`, `comments`) VALUES


