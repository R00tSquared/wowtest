-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- c5

insert into arena_team select * from characters3_import.arena_team;
insert into arena_team_member select * from characters3_import.arena_team_member;
insert into arena_team_stats select * from characters3_import.arena_team_stats;

insert into auction select * from characters3_import.auctionhouse;

insert into character_action (guid,spec,button,action,type) select guid,spec,button,action,type from characters3_import.character_action;

insert into character_gifts select * from characters3_import.character_gifts;
insert into character_homebind select * from characters3_import.character_homebind;
insert into character_inventory select * from characters3_import.character_inventory;

insert into character_queststatus_daily (guid,quest) select guid,quest from characters3_import.character_queststatus_daily;

insert into character_reputation select * from characters3_import.character_reputation;
insert into character_social select * from characters3_import.character_social;

insert into character_spell (guid,spell,active,disabled) select guid,spell,active,disabled from characters3_import.character_spell;

drop temporary table if exists tut;
create temporary table tut select * from characters3_import.character_tutorial;
alter table tut drop column realmid;
insert ignore into character_tutorial select * from tut;

insert into guild (guildid,name,leaderguid,EmblemStyle,EmblemColor,BorderStyle,BorderColor,BackgroundColor,info,motd,createdate,Bankmoney) select guildid,name,leaderguid,EmblemStyle,EmblemColor,BorderStyle,BorderColor,BackgroundColor,info,motd,createdate,Bankmoney from characters3_import.guild;

update guild set createdate = UNIX_TIMESTAMP(STR_TO_DATE(createdate, '%Y%m%d%H%i%s'));

insert into guild_bank_eventlog select * from characters3_import.guild_bank_eventlog;
insert into guild_bank_item select * from characters3_import.guild_bank_item;
insert into guild_bank_right select * from characters3_import.guild_bank_right;
insert into guild_bank_tab select * from characters3_import.guild_bank_tab;
insert into guild_eventlog select * from characters3_import.guild_eventlog;
insert into guild_member select * from characters3_import.guild_member;
insert into guild_rank select * from characters3_import.guild_rank;

insert into item_text select * from characters3_import.item_text;
insert into mail select * from characters3_import.mail;
insert into mail_external select * from characters3_import.mail_external;
insert into mail_items select * from characters3_import.mail_items;

-- insert into pet_spell (guid,spell,active) select guid,spell,active from characters3_import.pet_spell;

insert into petition select * from characters3_import.petition;
insert into petition_sign select * from characters3_import.petition_sign;

update saved_variables set NextArenaPointDistributionTime = (select NextArenaPointDistributionTime from characters3_import.saved_variables);

insert into character_raidchest select * from characters3_import.character_raidchest;
insert into character_talent select * from characters3_import.character_talent;
insert into character_transmogrification select * from characters3_import.character_transmogrification;

insert into shop select * from characters3_import.shop;
insert into shop_items select * from characters3_import.shop_items;

insert into character_queststatus select * from characters3_import.character_queststatus;

-- OBJECT_FIELD_GUID                         = 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC 1 2
-- OBJECT_FIELD_TYPE                         = 0x0002, // Size: 1, Type: INT, Flags: PUBLIC 3
-- OBJECT_FIELD_ENTRY                        = 0x0003, // Size: 1, Type: INT, Flags: PUBLIC 4
-- OBJECT_FIELD_SCALE_X                      = 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC 5
-- OBJECT_FIELD_PADDING                      = 0x0005, // Size: 1, Type: INT, Flags: NONE 6
-- ITEM_FIELD_OWNER                          = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC 7 8
-- ITEM_FIELD_CONTAINED                      = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC 9 10
-- ITEM_FIELD_CREATOR                        = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PUBLIC 11 12
-- ITEM_FIELD_GIFTCREATOR                    = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC 13 14
-- ITEM_FIELD_STACK_COUNT                    = OBJECT_END + 0x0008, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2 15
-- ITEM_FIELD_DURATION                       = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2 16
-- ITEM_FIELD_SPELL_CHARGES                  = OBJECT_END + 0x000A, // Size: 5, Type: INT, Flags: OWNER_ONLY, UNK2 17 18 19 20 21
-- ITEM_FIELD_FLAGS                          = OBJECT_END + 0x000F, // Size: 1, Type: INT, Flags: PUBLIC 22
-- ITEM_FIELD_ENCHANTMENT_1_1                = OBJECT_END + 0x0010, // Size: 33, Type: INT, Flags: PUBLIC 23-56
-- ITEM_FIELD_PROPERTY_SEED                  = OBJECT_END + 0x0031, // Size: 1, Type: INT, Flags: PUBLIC 57
-- ITEM_FIELD_RANDOM_PROPERTIES_ID           = OBJECT_END + 0x0032, // Size: 1, Type: INT, Flags: PUBLIC 58
-- ITEM_FIELD_ITEM_TEXT_ID                   = OBJECT_END + 0x0033, // Size: 1, Type: INT, Flags: OWNER_ONLY 59
-- ITEM_FIELD_DURABILITY                     = OBJECT_END + 0x0034, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2 60
-- ITEM_FIELD_MAXDURABILITY                  = OBJECT_END + 0x0035, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2 61

drop table item_instance;
CREATE TABLE `item_instance` (
  `guid` int(11) unsigned NOT NULL DEFAULT 0,
  `owner_guid` int(11) unsigned NOT NULL DEFAULT 0,
  `data` longtext DEFAULT NULL,
  PRIMARY KEY (`guid`),
  KEY `owner_guid` (`owner_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
insert into item_instance select * from characters3_import.item_instance;


CREATE TABLE item_instance_backup_pre_data_field_drop AS (SELECT * FROM item_instance);

ALTER TABLE `item_instance`
 ADD `itemEntry` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `owner_guid`,
 ADD `creatorGuid` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `itemEntry`,
 ADD `giftCreatorGuid` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `creatorGuid`,
 ADD `count` INT(10) UNSIGNED NOT NULL DEFAULT '1' AFTER `giftCreatorGuid`,
 ADD `duration` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `count`,
 ADD `charges` TEXT NOT NULL AFTER `duration`,
 ADD `flags` INT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `charges`,
 ADD `enchantments` TEXT NOT NULL AFTER `flags`,
 ADD `randomPropertyId` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `enchantments`,
 ADD `durability` INT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `randomPropertyId`,
 ADD `itemTextId` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `durability`;
 
 -- Temporarily change delimiter to prevent SQL syntax errors
DELIMITER ||

-- Function to convert ints from unsigned to signed
DROP FUNCTION IF EXISTS `uint32toint32`||
CREATE FUNCTION `uint32toint32`(input INT(10) UNSIGNED) RETURNS BIGINT(20) SIGNED DETERMINISTIC
BEGIN
  RETURN CAST((input<<32) AS SIGNED)/(1<<32);
END||

-- Restore original delimiter
DELIMITER ;

-- Move data to new fields
UPDATE `item_instance` SET
`itemEntry` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',3))+2, length(SUBSTRING_INDEX(`data`,' ',3+1))-length(SUBSTRING_INDEX(data,' ',3))-1),
`creatorGuid` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',10))+2, length(SUBSTRING_INDEX(`data`,' ',10+1))-length(SUBSTRING_INDEX(data,' ',10))-1),
`giftCreatorGuid` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',12))+2, length(SUBSTRING_INDEX(`data`,' ',12+1))-length(SUBSTRING_INDEX(data,' ',12))-1),
`count` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',14))+2, length(SUBSTRING_INDEX(`data`,' ',14+1))-length(SUBSTRING_INDEX(data,' ',14))-1),
`duration` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',15))+2, length(SUBSTRING_INDEX(`data`,' ',15+1))-length(SUBSTRING_INDEX(data,' ',15))-1),
`charges` = CONCAT_WS(' ',
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',16))+2, length(SUBSTRING_INDEX(`data`,' ',16+1))-length(SUBSTRING_INDEX(data,' ',16))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',17))+2, length(SUBSTRING_INDEX(`data`,' ',17+1))-length(SUBSTRING_INDEX(data,' ',17))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',18))+2, length(SUBSTRING_INDEX(`data`,' ',18+1))-length(SUBSTRING_INDEX(data,' ',18))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',19))+2, length(SUBSTRING_INDEX(`data`,' ',19+1))-length(SUBSTRING_INDEX(data,' ',19))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',20))+2, length(SUBSTRING_INDEX(`data`,' ',20+1))-length(SUBSTRING_INDEX(data,' ',20))-1)) ),
`flags` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',21))+2, length(SUBSTRING_INDEX(`data`,' ',21+1))-length(SUBSTRING_INDEX(data,' ',21))-1),
`enchantments` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',22))+2, length(SUBSTRING_INDEX(`data`,' ',54+1))-length(SUBSTRING_INDEX(data,' ',22))-1),
`randomPropertyId` = uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',56))+2, length(SUBSTRING_INDEX(`data`,' ',56+1))-length(SUBSTRING_INDEX(data,' ',56))-1)),
`durability` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',58))+2, length(SUBSTRING_INDEX(`data`,' ',58+1))-length(SUBSTRING_INDEX(data,' ',58))-1),
`itemTextId` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',57))+2, length(SUBSTRING_INDEX(`data`,' ',57+1))-length(SUBSTRING_INDEX(data,' ',57))-1);

-- Drop function
DROP FUNCTION IF EXISTS `uint32toint32`;

-- Drop old field
ALTER TABLE `item_instance` DROP `data`;


-- characters

create temporary table char_del select guid from characters3_import.characters where account<10000000 and account>2;
insert into char_del select guid from characters where name in ('Guildru','Guilden');

insert into characters (
  guid,
  account,
  name,
  race,
  class,
  gender,
  level,
  xp,
  money,
  playerBytes,
  playerBytes2,
  playerFlags,
  position_x,
  position_y,
  position_z,
  map,
  dungeon_difficulty,
  orientation,
  taximask,
  online,
  cinematic,
  totaltime,
  leveltime,
  logout_time,
  is_logout_resting,
  rest_bonus,
  resettalents_cost,
  resettalents_time,
  trans_x,
  trans_y,
  trans_z,
  trans_o,
  transguid,
  extra_flags,
  stable_slots,
  at_login,
  zone,
  death_expire_time,
  taxi_path,
  arenaPoints,
  totalHonorPoints,
  todayHonorPoints,
  yesterdayHonorPoints,
  totalKills,
  todayKills,
  yesterdayKills,
  chosenTitle,
  watchedFaction,
  drunk,
  health,
  power1,
  power2,
  power3,
  power4,
  power5,
  exploredZones,
  equipmentCache,
  ammoId,
  knownTitles,
  actionBars,
  grantableLevels,
  fishingSteps,
  deleteInfos_Account,
  deleteInfos_Name,
  deleteDate,
  activeSpec
)

select 
-- guid,
guid,
-- account,
account,
-- name,
name,
-- race,
race,
-- class,
class,
-- gender,
gender,
-- level,
level,
-- xp,
xp,
-- money,
money,
-- playerBytes,
playerBytes,
-- playerBytes2,
playerBytes2,
-- playerFlags,
playerFlags,
-- position_x,
position_x,
-- position_y,
position_y,
-- position_z,
position_z,
-- map,
map,
-- dungeon_difficulty,
dungeon_difficulty,
-- orientation,
orientation,
-- taximask,
taximask,
-- online,
online,
-- cinematic,
cinematic,
-- totaltime,
totaltime,
-- leveltime,
leveltime,
-- logout_time,
logout_time,
-- is_logout_resting,
is_logout_resting,
-- rest_bonus,
rest_bonus,
-- resettalents_cost,
resettalents_cost,
-- resettalents_time,
resettalents_time,
-- trans_x,
trans_x,
-- trans_y,
trans_y,
-- trans_z,
trans_z,
-- trans_o,
trans_o,
-- transguid,
transguid,
-- extra_flags,
extra_flags,
-- stable_slots,
stable_slots,
-- at_login,
at_login,
-- zone,
zone,
-- death_expire_time,
death_expire_time,
-- taxi_path,
taxi_path,
-- arenaPoints,
arenaPoints,
-- totalHonorPoints,
totalHonorPoints,
-- todayHonorPoints,
0,
-- yesterdayHonorPoints,
0,
-- totalKills,
totalKills,
-- todayKills,
todayKills,
-- yesterdayKills,
0,
-- chosenTitle,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',  648))+2, length(SUBSTRING_INDEX(data, ' ',  648+1))- length(SUBSTRING_INDEX(data, ' ',  648)) - 1),
-- watchedFaction,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1519))+2, length(SUBSTRING_INDEX(data, ' ', 1519+1))- length(SUBSTRING_INDEX(data, ' ', 1519)) - 1),
-- drunk,
0,
-- health,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   22))+2, length(SUBSTRING_INDEX(data, ' ',   22+1))- length(SUBSTRING_INDEX(data, ' ',   22)) - 1),
  -- power1,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   23))+2, length(SUBSTRING_INDEX(data, ' ',   23+1))- length(SUBSTRING_INDEX(data, ' ',   23)) - 1),
-- power2,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   24))+2, length(SUBSTRING_INDEX(data, ' ',   24+1))- length(SUBSTRING_INDEX(data, ' ',   24)) - 1),
-- power3,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   25))+2, length(SUBSTRING_INDEX(data, ' ',   25+1))- length(SUBSTRING_INDEX(data, ' ',   25)) - 1),
-- power4,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   26))+2, length(SUBSTRING_INDEX(data, ' ',   26+1))- length(SUBSTRING_INDEX(data, ' ',   26)) - 1),
-- power5,
SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   27))+2, length(SUBSTRING_INDEX(data, ' ',   27+1))- length(SUBSTRING_INDEX(data, ' ',   27)) - 1),
 -- exploredZones,
SUBSTRING(data,length(SUBSTRING_INDEX(data, ' ', 1332))+2,length(SUBSTRING_INDEX(data, ' ', 1459+1))- length(SUBSTRING_INDEX(data, ' ', 1332)) - 1),
-- equipmentCache,
'0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ' as equipmentCache,
-- ammoId,
TRIM(SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ', 1519-31), ' ', -1)),
-- knownTitles,
SUBSTRING(data,length(SUBSTRING_INDEX(data, ' ', 924))+2,length(SUBSTRING_INDEX(data, ' ', 925+1))- length(SUBSTRING_INDEX(data, ' ', 924)) - 1),
-- actionBars,
((SUBSTRING(data,length(SUBSTRING_INDEX(data, ' ', 1486))+2,length(SUBSTRING_INDEX(data, ' ', 1486+1))- length(SUBSTRING_INDEX(data, ' ', 1486)) - 1) & 0xFF0000) >> 16),
-- grantableLevels,
0,
-- fishingSteps,
0,
-- deleteInfos_Account,
NULL,
-- deleteInfos_Name,
NULL,
-- deleteDate,
NULL,
-- activeSpec
activeSpec 
from characters3_import.characters where guid in (select guid from char_del);

alter table character_inventory add KEY `idx_item_template` (`item_template`);
alter table item_instance add KEY `idx_itemEntry` (`itemEntry`);


-- character_skills
DROP TABLE IF EXISTS `character_skills`;
CREATE TABLE `character_skills` (
  `guid` int(11) unsigned NOT NULL COMMENT 'Global Unique Identifier',
  `skill` mediumint(9) unsigned NOT NULL,
  `value` int(11) unsigned NOT NULL,
  `max` mediumint(9) unsigned NOT NULL,
  i mediumint(9),
  PRIMARY KEY  (`guid`,`skill`,`i`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

DROP TABLE IF EXISTS temp_skills;
CREATE TABLE temp_skills (
  i int(11) unsigned NOT NULL,
  PRIMARY KEY (i)
);

INSERT INTO temp_skills VALUES
( 0),( 1),( 2),( 3),( 4),( 5),( 6),( 7),( 8),( 9),(10),(11),(12),(13),(14),(15),(16),(17),(18),(19),
(20),(21),(22),(23),(24),(25),(26),(27),(28),(29),(30),(31),(32),(33),(34),(35),(36),(37),(38),(39),
(40),(41),(42),(43),(44),(45),(46),(47),(48),(49),(50),(51),(52),(53),(54),(55),(56),(57),(58),(59),
(60),(61),(62),(63),(64),(65),(66),(67),(68),(69),(70),(71),(72),(73),(74),(75),(76),(77),(78),(79),
(80),(81),(82),(83),(84),(85),(86),(87),(88),(89),(90),(91),(92),(93),(94),(95),(96),(97),(98),(99),
(100),(101),(102),(103),(104),(105),(106),(107),(108),(109),(110),(111),(112),(113),(114),(115),(116),(117),(118),(119),
(120),(121),(122),(123),(124),(125),(126),(127);

INSERT INTO character_skills SELECT
guid,
((SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 928+3*i))+2, length(SUBSTRING_INDEX(data, ' ', 928+3*i+1))- length(SUBSTRING_INDEX(data, ' ', 928+3*i)) - 1)) & 0xFFFF) as skill,
(SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 928+3*i+1))+2, length(SUBSTRING_INDEX(data, ' ', 928+3*i+2))- length(SUBSTRING_INDEX(data, ' ', 928+3*i+1)) - 1)) as value,
(0) as max,
i
FROM characters3_import.characters, temp_skills;

DELETE FROM character_skills WHERE skill = 0;
DROP TABLE IF EXISTS temp_skills;

UPDATE character_skills
  SET max = ((value & 0xFFFF0000) >> 16);

UPDATE character_skills
  SET value = (value & 0xFFFF);

ALTER IGNORE TABLE character_skills
  CHANGE COLUMN value value mediumint(9) unsigned NOT NULL,
  DROP PRIMARY KEY,
  ADD  PRIMARY KEY (guid,skill),
  DROP COLUMN i;


-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
-- cleanups
delete from character_skills where guid not in (select distinct guid from characters);

UPDATE `character_skills` SET `value` = `max` WHERE `value` > `max`;
DELETE a FROM character_skills AS a WHERE skill IN (118) AND guid IN (SELECT guid FROM characters b WHERE class IN (4) AND level < 10 AND a.guid=b.guid);
DELETE a FROM character_skills AS a WHERE skill IN (760) AND guid IN (SELECT guid FROM characters b WHERE race IN (5) AND a.guid=b.guid);

DELETE FROM character_spell WHERE spell in (33876,33878,33982,33983,33986,33987,48563,48564,48565,48566);

UPDATE characters SET at_login = at_login | 0x4;
UPDATE characters SET activeSpec = 0;
delete from character_talent;



DELETE FROM `character_spell` WHERE `spell` IN (7376,3025,5419,5421,21156,7381,1178,21178,9635,21178,24905,5420,34123,33948,34764,40121,40122);

DELETE FROM character_spell WHERE `spell` IN (
 20580, /*old Shadowmeld*/
 20600, /*Perception*/
 21009, /*old Shadowmeld Passive and new Elusiveness (learned as racial passive)*/
 21184  /*old Seal of Righteousness*/
);

/*old Shadow Resistance, leaned as racial passive of race 5 */
DELETE FROM character_spell USING character_spell INNER JOIN characters ON character_spell.guid = characters.guid
WHERE character_spell.spell = 20579 AND characters.race <> 5;

UPDATE IGNORE character_spell SET spell = 2018 WHERE spell = 2020;
DELETE FROM character_spell                    WHERE spell = 2020;

UPDATE IGNORE character_spell SET spell = 2018 WHERE spell = 2020;
DELETE FROM character_spell                    WHERE spell = 2020;

UPDATE IGNORE character_spell SET spell = 3100 WHERE spell = 2021;
DELETE FROM character_spell                    WHERE spell = 2021;

UPDATE IGNORE character_spell SET spell = 3104 WHERE spell = 2154;
DELETE FROM character_spell                    WHERE spell = 2154;

UPDATE IGNORE character_spell SET spell = 2108 WHERE spell = 2155;
DELETE FROM character_spell                    WHERE spell = 2155;

UPDATE IGNORE character_spell SET spell = 2259 WHERE spell = 2275;
DELETE FROM character_spell                    WHERE spell = 2275;

UPDATE IGNORE character_spell SET spell = 3101 WHERE spell = 2280;
DELETE FROM character_spell                    WHERE spell = 2280;

UPDATE IGNORE character_spell SET spell = 2366 WHERE spell = 2372;
DELETE FROM character_spell                    WHERE spell = 2372;

UPDATE IGNORE character_spell SET spell = 2368 WHERE spell = 2373;
DELETE FROM character_spell                    WHERE spell = 2373;

UPDATE IGNORE character_spell SET spell = 2550 WHERE spell = 2551;
DELETE FROM character_spell                    WHERE spell = 2551;

UPDATE IGNORE character_spell SET spell = 2575 WHERE spell = 2581;
DELETE FROM character_spell                    WHERE spell = 2581;

UPDATE IGNORE character_spell SET spell = 2576 WHERE spell = 2582;
DELETE FROM character_spell                    WHERE spell = 2582;

UPDATE IGNORE character_spell SET spell = 3273 WHERE spell = 3279;
DELETE FROM character_spell                    WHERE spell = 3279;

UPDATE IGNORE character_spell SET spell = 3274 WHERE spell = 3280;
DELETE FROM character_spell                    WHERE spell = 3280;

UPDATE IGNORE character_spell SET spell = 3102 WHERE spell = 3412;
DELETE FROM character_spell                    WHERE spell = 3412;

UPDATE IGNORE character_spell SET spell = 3464 WHERE spell = 3465;
DELETE FROM character_spell                    WHERE spell = 3465;

UPDATE IGNORE character_spell SET spell = 3538 WHERE spell = 3539;
DELETE FROM character_spell                    WHERE spell = 3539;

UPDATE IGNORE character_spell SET spell = 3564 WHERE spell = 3568;
DELETE FROM character_spell                    WHERE spell = 3568;

UPDATE IGNORE character_spell SET spell = 3570 WHERE spell = 3571;
DELETE FROM character_spell                    WHERE spell = 3571;

UPDATE IGNORE character_spell SET spell = 3811 WHERE spell = 3812;
DELETE FROM character_spell                    WHERE spell = 3812;

UPDATE IGNORE character_spell SET spell = 3908 WHERE spell = 3911;
DELETE FROM character_spell                    WHERE spell = 3911;

UPDATE IGNORE character_spell SET spell = 3909 WHERE spell = 3912;
DELETE FROM character_spell                    WHERE spell = 3912;

UPDATE IGNORE character_spell SET spell = 3910 WHERE spell = 3913;
DELETE FROM character_spell                    WHERE spell = 3913;

UPDATE IGNORE character_spell SET spell = 4036 WHERE spell = 4039;
DELETE FROM character_spell                    WHERE spell = 4039;

UPDATE IGNORE character_spell SET spell = 4037 WHERE spell = 4040;
DELETE FROM character_spell                    WHERE spell = 4040;

UPDATE IGNORE character_spell SET spell = 4038 WHERE spell = 4041;
DELETE FROM character_spell                    WHERE spell = 4041;

UPDATE IGNORE character_spell SET spell = 7620 WHERE spell = 7733;
DELETE FROM character_spell                    WHERE spell = 7733;

UPDATE IGNORE character_spell SET spell = 7731 WHERE spell = 7734;
DELETE FROM character_spell                    WHERE spell = 7734;

UPDATE IGNORE character_spell SET spell = 8613 WHERE spell = 8615;
DELETE FROM character_spell                    WHERE spell = 8615;

UPDATE IGNORE character_spell SET spell = 8617 WHERE spell = 8619;
DELETE FROM character_spell                    WHERE spell = 8619;

UPDATE IGNORE character_spell SET spell = 8618 WHERE spell = 8620;
DELETE FROM character_spell                    WHERE spell = 8620;

UPDATE IGNORE character_spell SET spell = 9785 WHERE spell = 9786;
DELETE FROM character_spell                    WHERE spell = 9786;

UPDATE IGNORE character_spell SET spell = 10248 WHERE spell = 10249;
DELETE FROM character_spell                     WHERE spell = 10249;

UPDATE IGNORE character_spell SET spell = 10662 WHERE spell = 10663;
DELETE FROM character_spell                     WHERE spell = 10663;

UPDATE IGNORE character_spell SET spell = 10768 WHERE spell = 10769;
DELETE FROM character_spell                     WHERE spell = 10769;

UPDATE IGNORE character_spell SET spell = 11611 WHERE spell = 11612;
DELETE FROM character_spell                     WHERE spell = 11612;

UPDATE IGNORE character_spell SET spell = 11993 WHERE spell = 11994;
DELETE FROM character_spell                     WHERE spell = 11994;

UPDATE IGNORE character_spell SET spell = 12180 WHERE spell = 12181;
DELETE FROM character_spell                     WHERE spell = 12181;

UPDATE IGNORE character_spell SET spell = 12656 WHERE spell = 12657;
DELETE FROM character_spell                     WHERE spell = 12657;

UPDATE IGNORE character_spell SET spell = 25229 WHERE spell = 25245;
DELETE FROM character_spell                     WHERE spell = 25245;

UPDATE IGNORE character_spell SET spell = 25230 WHERE spell = 25246;
DELETE FROM character_spell                     WHERE spell = 25246;

UPDATE IGNORE character_spell SET spell = 26790 WHERE spell = 26791;
DELETE FROM character_spell                     WHERE spell = 26791;

UPDATE IGNORE character_spell SET spell = 28596 WHERE spell = 28597;
DELETE FROM character_spell                     WHERE spell = 28597;

UPDATE IGNORE character_spell SET spell = 28695 WHERE spell = 28696;
DELETE FROM character_spell                     WHERE spell = 28696;

UPDATE IGNORE character_spell SET spell = 28894 WHERE spell = 28896;
DELETE FROM character_spell                     WHERE spell = 28896;

UPDATE IGNORE character_spell SET spell = 28895 WHERE spell = 28899;
DELETE FROM character_spell                     WHERE spell = 28899;

UPDATE IGNORE character_spell SET spell = 28897 WHERE spell = 28901;
DELETE FROM character_spell                     WHERE spell = 28901;

UPDATE IGNORE character_spell SET spell = 29354 WHERE spell = 29355;
DELETE FROM character_spell                     WHERE spell = 29355;

UPDATE IGNORE character_spell SET spell = 29844 WHERE spell = 29845;
DELETE FROM character_spell                     WHERE spell = 29845;

UPDATE IGNORE character_spell SET spell = 30350 WHERE spell = 30351;
DELETE FROM character_spell                     WHERE spell = 30351;

UPDATE IGNORE character_spell SET spell = 32549 WHERE spell = 32550;
DELETE FROM character_spell                     WHERE spell = 32550;

UPDATE IGNORE character_spell SET spell = 32678 WHERE spell = 32679;
DELETE FROM character_spell                     WHERE spell = 32679;

UPDATE IGNORE character_spell SET spell = 45357 WHERE spell = 45375;
DELETE FROM character_spell                     WHERE spell = 45375;

UPDATE IGNORE character_spell SET spell = 45358 WHERE spell = 45376;
DELETE FROM character_spell                     WHERE spell = 45376;

UPDATE IGNORE character_spell SET spell = 45359 WHERE spell = 45377;
DELETE FROM character_spell                     WHERE spell = 45377;

UPDATE IGNORE character_spell SET spell = 45360 WHERE spell = 45378;
DELETE FROM character_spell                     WHERE spell = 45378;

UPDATE IGNORE character_spell SET spell = 45361 WHERE spell = 45379;
DELETE FROM character_spell                     WHERE spell = 45379;

UPDATE IGNORE character_spell SET spell = 45363 WHERE spell = 45380;
DELETE FROM character_spell                     WHERE spell = 45380;

UPDATE IGNORE character_spell SET spell = 45542 WHERE spell = 50299;
DELETE FROM character_spell                     WHERE spell = 50299;

UPDATE IGNORE character_spell SET spell = 50305 WHERE spell = 50307;
DELETE FROM character_spell                     WHERE spell = 50307;

UPDATE IGNORE character_spell SET spell = 50310 WHERE spell = 50309;
DELETE FROM character_spell                     WHERE spell = 50309;

UPDATE IGNORE character_spell SET spell = 51294 WHERE spell = 51293;
DELETE FROM character_spell                     WHERE spell = 51293;

UPDATE IGNORE character_spell SET spell = 51296 WHERE spell = 51295;
DELETE FROM character_spell                     WHERE spell = 51295;

UPDATE IGNORE character_spell SET spell = 51300 WHERE spell = 51298;
DELETE FROM character_spell                     WHERE spell = 51298;

UPDATE IGNORE character_spell SET spell = 51302 WHERE spell = 51301;
DELETE FROM character_spell                     WHERE spell = 51301;

UPDATE IGNORE character_spell SET spell = 51304 WHERE spell = 51303;
DELETE FROM character_spell                     WHERE spell = 51303;

UPDATE IGNORE character_spell SET spell = 51306 WHERE spell = 51305;
DELETE FROM character_spell                     WHERE spell = 51305;

UPDATE IGNORE character_spell SET spell = 51309 WHERE spell = 51308;
DELETE FROM character_spell                     WHERE spell = 51308;

UPDATE IGNORE character_spell SET spell = 51311 WHERE spell = 51310;
DELETE FROM character_spell                     WHERE spell = 51310;

UPDATE IGNORE character_spell SET spell = 51313 WHERE spell = 51312;
DELETE FROM character_spell                     WHERE spell = 51312;

UPDATE IGNORE character_spell SET spell = 33095 WHERE spell = 54084;
DELETE FROM character_spell                     WHERE spell = 54084;

/* Warrior cleanup */
DELETE FROM `character_spell` WHERE `spell` IN (1715,7372,7373);                  /* Hamstring old */
DELETE FROM `character_spell` WHERE `spell` IN (72,17671,1672);                   /* Mortar Disturb old */
DELETE FROM `character_spell` WHERE `spell` IN (7384,7887,11584,11586);           /* Overpower old */
DELETE FROM `character_spell` WHERE `spell`=23881;                                /* Bloodthirst old */
DELETE FROM `character_spell` WHERE `spell` IN (6552,6554);                       /* Pummel old */
DELETE FROM `character_spell` WHERE `spell` IN (694,7400,7402,20559,20560,25266); /* Mocking Blow old */
/* Druid cleanup */
DELETE FROM `character_spell` WHERE `spell`=22842;                                /* Frenzied Regeneration old */
/* Hunter cleanup */
DELETE FROM `character_spell` WHERE `spell`=14268;                                /* Wing Clip r1 old */
DELETE FROM `character_spell` WHERE `spell`=14267;                                /* Wing Clip r2 old */
/* Rogue */
DELETE FROM `character_spell` WHERE `spell` IN (1766,1767,1768,1769,38768);       /* Kick old */
DELETE FROM `character_spell` WHERE `spell` IN (1776,1777,8629,11285,11286,38764);/* Gouge old */
DELETE FROM `character_spell` WHERE `spell`=2842;                                 /* Poisons old */

/* Hunter's training spells for pets */
DELETE FROM `character_spell` WHERE `spell` IN (2949,2975,2976,2977,2980,2981,2982,3666,3667,4630,6327,6359,6362,
    7370,7832,7833,7834,7835,7871,7872,7873,7876,7877,7878,7879,7880,7881,7882,7883,7884,7885,7886,8318,8319,11764,
    11765,11768,11769,11772,11773,11776,11777,11781,11782,11783,11786,11787,17254,17262,17263,17264,17265,17266,
    17267,17268,17736,17753,17754,17755,17776,17855,17856,17857,17859,17860,19439,19444,19445,19446,19447,19481,
    19577,19648,19650,19661,19662,19663,19664,19737,19738,19739,20270,20312,20313,20314,20315,20316,20317,20318,
    20319,20320,20321,20322,20323,20324,20326,20327,20329,20377,20378,20379,20380,20381,20382,20383,20384,20385,
    20386,20387,20388,20389,20390,20391,20392,20393,20394,20395,20396,20397,20398,20399,20400,20401,20402,20403,
    20404,20405,20406,20407,20408,20426,20427,20428,20429,20430,20431,20432,20433,20434,20435,23100,23111,23112,
    23146,23149,23150,24424,24440,24441,24451,24454,24455,24463,24464,24475,24476,24477,24580,24581,24582,24584,
    24588,24589,24599,24607,24608,24609,24641,26065,26094,26184,26185,26186,26189,26190,26202,27347,27348,27349,
    27361,27366,27484,27485,27486,27487,27488,27489,27490,27491,27492,27493,27494,27495,27496,27497,27500,28343,
    33703,35299,35300,35302,35303,35304,35305,35306,35307,35308);
DELETE FROM `character_spell` WHERE `spell` IN (1853,14922,14923,14924,14925,14926,14927,27344);
DELETE FROM `character_spell` WHERE `spell` IN (27353,24516,24515,24514,24490);
DELETE FROM `character_spell` WHERE `spell` IN (27354,24513,24512,24511,24494,2119);
UPDATE IGNORE character_spell SET spell = 2108 WHERE spell = 3104;
DELETE FROM character_spell                    WHERE spell = 3104;
/* This cleanup character_action. This is like delete from character_action where type=0 and action not in character_spell for same player */
DELETE FROM ca,cs USING `character_action` ca LEFT JOIN `character_spell` cs ON ca.`guid`=cs.`guid` AND ca.`action`=cs.`spell` WHERE ca.`type`=0 AND cs.`guid` IS NULL;

DELETE FROM `character_spell` WHERE `spell` IN (52375,47541);

/* remove some deleted spells or ranks from characters */
/* Mana Tap no longer Blood Elf Racial */
DELETE FROM `character_spell` WHERE `spell` = '28734';
/* Hamstring is only one rank now, need to delete these zzOLDRank spells */
DELETE FROM `character_spell` WHERE `spell` IN ('7373', '7372', '25212');
/* Intercept is only one rank now, need to delete these zzOLDRank spells */
DELETE FROM `character_spell` WHERE `spell` IN ('20616', '20617', '25272', '25275');
/* Overpower is only one rank now, need to delete these zzOLDRank spells */
DELETE FROM `character_spell` WHERE `spell` IN ('7887', '11584', '11585');
/* Shield Bash is only one rank now, need to delete these zzOLDRank spells */
DELETE FROM `character_spell` WHERE `spell` IN ('1671', '1672', '29704');

DELETE FROM character_spell WHERE spell = '28734';
DELETE FROM  character_action WHERE action = '28734' AND type = '0';

-- 0.13 -> 0.14

UPDATE IGNORE character_spell
  SET spell = 64901
  WHERE spell = 64904;

DELETE FROM character_spell WHERE spell = 64904;

UPDATE character_action
  SET action = 64901
  WHERE action = 64904 AND type = '0';

UPDATE IGNORE character_spell SET spell=7386 WHERE spell IN (7405,8380,11596,11597,25225,47467);
UPDATE character_spell SET active=1 WHERE spell=7386;
DELETE FROM character_spell WHERE spell IN (7405,8380,11596,11597,25225,47467);

UPDATE character_spell SET active=1 WHERE spell=16857;

DELETE FROM `character_spell` WHERE `spell` IN (31892, 53720);
DELETE FROM `character_action` WHERE `action` IN (31892, 53720) AND `type`=0;