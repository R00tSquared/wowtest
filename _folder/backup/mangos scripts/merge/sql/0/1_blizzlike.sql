-- Wolf under textures
UPDATE `creature` SET `position_z` = 5.63 WHERE `guid` = 66566;

-- Kalecgos ignore mmaps
UPDATE `creature_template` SET `ExtraFlags` = 16384 WHERE `Entry` = 24844;

-- Brutallus detection range
UPDATE `creature_template` SET `Detection` = 30 WHERE `Entry` = 24882;