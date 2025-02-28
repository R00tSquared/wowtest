-- seals for paladins
delete from npc_trainer_template where spell in (31801,31892);

INSERT INTO `npc_trainer_template` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `condition_id`) VALUES (21, 31801, 67000, 0, 0, 64, NULL, NULL, NULL, 0);
INSERT INTO `npc_trainer_template` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `condition_id`) VALUES (22, 31801, 67000, 0, 0, 64, NULL, NULL, NULL, 0);
INSERT INTO `npc_trainer_template` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `condition_id`) VALUES (2010, 31801, 67000, 0, 0, 64, NULL, NULL, NULL, 0);

INSERT INTO `npc_trainer_template` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `condition_id`) VALUES (21, 31892, 50000, 0, 0, 64, NULL, NULL, NULL, 0);
INSERT INTO `npc_trainer_template` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `condition_id`) VALUES (22, 31892, 50000, 0, 0, 64, NULL, NULL, NULL, 0);
INSERT INTO `npc_trainer_template` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `condition_id`) VALUES (2010, 31892, 50000, 0, 0, 64, NULL, NULL, NULL, 0);

-- warden checks
delete from warden_scans where id in (45,62);