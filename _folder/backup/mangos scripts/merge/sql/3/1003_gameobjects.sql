-- portals
replace into `spell_target_position` (`id`, `target_map`, `target_position_x`, `target_position_y`, `target_position_z`, `target_orientation`) VALUES (35718, 530, -1975.73, 5504.21, -12.42, 5.96);
replace INTO `spell_target_position` (`id`, `target_map`, `target_position_x`, `target_position_y`, `target_position_z`, `target_orientation`) VALUES (17334, 0, -8928.67, 541.19, 94.31, 2.28);
replace INTO `spell_target_position` (`id`, `target_map`, `target_position_x`, `target_position_y`, `target_position_z`, `target_orientation`) VALUES (17609, 1, 1436, -4425.02, 25.23, 5.98);

-- remove doors to access Karazhan and etc.
delete from gameobject where guid in (14041);