-- this was used to make rates lower for certain valuable items

-- item - recipe
-- 34380 - 35208
-- 34378 - 35209
-- 34361 - 34361
-- 34362 - 35198
-- 34363 - 35199
-- 34364 - 35206
-- 34366 - 35204
-- 34367 - 35205
-- 34370 - 35214
-- 34372 - 35212
-- 34374 - 35213
-- 34376 - 35215

-- mysql> select * from reference_loot_template where item in (35208,35209,34361,35198,35199,35206,35204,35205,35214,35212,35213,35215);
-- +-------+-------+---------------------+---------+---------------+----------+--------------+-------------------------------------------+
-- | entry | item  | ChanceOrQuestChance | groupid | mincountOrRef | maxcount | condition_id | comments                                  |
-- +-------+-------+---------------------+---------+---------------+----------+--------------+-------------------------------------------+
-- | 36201 | 35206 |                 4.5 |       1 |             1 |        1 |            0 | Pattern: Sunfire Robe                     |
-- | 36201 | 35215 |                   3 |       1 |             1 |        1 |          199 | Pattern: Sun-Drenched Scale Gloves        |
-- | 36201 | 35214 |                   3 |       1 |             1 |        1 |          199 | Pattern: Gloves of Immortal Dusk          |
-- | 36201 | 35213 |                   3 |       1 |             1 |        1 |          199 | Pattern: Fletcher's Gloves of the Phoenix |
-- | 36201 | 35212 |                   3 |       1 |             1 |        1 |          199 | Pattern: Leather Gauntlets of the Sun     |
-- | 36201 | 35209 |                   3 |       1 |             1 |        1 |          198 | Plans: Hard Khorium Battlefists           |
-- | 36201 | 35208 |                   3 |       1 |             1 |        1 |          198 | Plans: Sunblessed Gauntlets               |
-- | 36201 | 35205 |                   3 |       1 |             1 |        1 |          201 | Pattern: Hands of Eternal Light           |
-- | 36201 | 35204 |                   3 |       1 |             1 |        1 |          201 | Pattern: Sunfire Handwraps                |
-- | 36201 | 35199 |                   3 |       1 |             1 |        1 |          204 | Design: Ring of Flowing Life              |
-- | 36201 | 35198 |                   3 |       1 |             1 |        1 |          204 | Design: Loop of Forged Power              |
-- +-------+-------+---------------------+---------+---------------+----------+--------------+-------------------------------------------+
update reference_loot_template set ChanceOrQuestChance = 0.01 WHERE item in (35208,35209,34361,35198,35199,35206,35204,35205,35214,35212,35213,35215);

-- select * from creature_loot_template where mincountorref in (-10005,-12005,-50001);
update creature_loot_template set ChanceOrQuestChance =  ChanceOrQuestChance / 10 WHERE mincountorref in (-10005,-12005,-50001);

-- trash items
-- 34183 34346 34347 34348 34349 34350 34351 35733
-- mysql> select * from reference_loot_template where item in (34183,34346,34347,34348,34349,34350,34351,35733);
-- +-------+-------+---------------------+---------+---------------+----------+--------------+-------------------------------------+
-- | entry | item  | ChanceOrQuestChance | groupid | mincountOrRef | maxcount | condition_id | comments                            |
-- +-------+-------+---------------------+---------+---------------+----------+--------------+-------------------------------------+
-- | 36200 | 35733 |                   0 |       1 |             1 |        1 |            0 | Ring of Harmonic Beauty             |
-- | 36200 | 34351 |                   0 |       1 |             1 |        1 |            0 | Tranquil Majesty Wraps              |
-- | 36200 | 34350 |                   0 |       1 |             1 |        1 |            0 | Gauntlets of the Ancient Shadowmoon |
-- | 36200 | 34349 |                   0 |       1 |             1 |        1 |            0 | Blade of Life's Inevitability       |
-- | 36200 | 34348 |                   0 |       1 |             1 |        1 |            0 | Wand of Cleansing Light             |
-- | 36200 | 34347 |                   0 |       1 |             1 |        1 |            0 | Wand of the Demonsoul               |
-- | 36200 | 34346 |                   0 |       1 |             1 |        1 |            0 | Mounting Vengeance                  |
-- | 36200 | 34183 |                   0 |       1 |             1 |        1 |            0 | Shivering Felspine                  |
-- +-------+-------+---------------------+---------+---------------+----------+--------------+-------------------------------------+
update creature_loot_template set ChanceOrQuestChance = ChanceOrQuestChance / 100 where mincountOrRef=-36200;

-- bt trash items 
-- 32527 32528 32526
-- +-------+-------+---------------------+---------+---------------+----------+--------------+---------------------------+
-- | entry | item  | ChanceOrQuestChance | groupid | mincountOrRef | maxcount | condition_id | comments                  |
-- +-------+-------+---------------------+---------+---------------+----------+--------------+---------------------------+
-- | 36198 | 32527 |                   0 |       1 |             1 |        1 |            0 | Ring of Ancient Knowledge |
-- | 36198 | 32526 |                   0 |       1 |             1 |        1 |            0 | Band of Devastation       |
-- | 36198 | 32528 |                   0 |       1 |             1 |        1 |            0 | Blessed Band of Karabor   |
-- +-------+-------+---------------------+---------+---------------+----------+--------------+---------------------------+
update creature_loot_template set ChanceOrQuestChance = ChanceOrQuestChance / 100 where mincountOrRef=-36198;

-- mounts
-- select * from reference_loot_template where item in (select entry from item_template where requiredskill=762);
-- select * from creature_loot_template where item in (select entry from item_template where requiredskill=762);
update reference_loot_template set ChanceOrQuestChance = ChanceOrQuestChance / 100 where item in (19902,19872);
update creature_loot_template set ChanceOrQuestChance = ChanceOrQuestChance / 100 where item in (37012,35513,13335,19029,37011,33183,33184,33182,33176,32768,30480,32458,37828,33977);

-- Archimonde 17968
DELETE FROM `creature_loot_template` WHERE `entry` = 17968 AND `item` = 2;
UPDATE reference_loot_template SET groupid=1 WHERE entry=36105;

-- Illidan 22917
DELETE FROM `creature_loot_template` WHERE `entry` = 22917 AND `item` = 2;
UPDATE reference_loot_template SET groupid=1 WHERE entry=36131;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 0.4 WHERE `entry` = 22917 AND `item` = 32837;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 0.4 WHERE `entry` = 22917 AND `item` = 32838;

-- 24892: Sathrovarr the Corruptor 
delete from creature_loot_template where entry=24892;
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 6209, 0, 2, 1, 1, 0, 'Legendary Key');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34170, 5.1, 2, 1, 1, 0, 'Pantaloons of Calming Strife');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34169, 5.1, 2, 1, 1, 0, 'Breeches of Natural Aggression');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34168, 5.1, 2, 1, 1, 0, 'Starstalker Legguards');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34167, 5.1, 2, 1, 1, 0, 'Legplates of the Holy Juggernaut');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34166, 5.1, 2, 1, 1, 0, 'Band of Lucent Beams');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34165, 5.1, 2, 1, 1, 0, 'Fang of Kalecgos');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34164, 5.1, 2, 1, 1, 0, 'Dragonscale-Encrusted Longblade');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 12005, 33, 1, -12005, 1, 0, 'Epic Gem - TBC');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 36171, 100, 1, -36171, 3, 0, 'Sunwell Plateau (Boss Loot) - Sathrovarr the Corruptor (24892) - Tokens (T6 Bracers)');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 33117, 0.5, 0, 1, 1, 6012, 'Jack-o\'-Lantern');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 29434, 100, 0, 2, 2, 0, 'Badge of Justice');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34544, -100, 0, 1, 1, 10333, 'Essence of the Immortals');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 34664, 15, 0, 1, 1, 0, 'Sunmote');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24892, 50501, 0.2, 0, -50501, 1, 0, 'NPC LOOT - Profession (-Design,-Formula,-Pattern,-Plans,-Recipe,-Schematic)(Non-BoP) - NPC Level 64+ Non-Elite/Level 58+ Elite - TBC NPC ONLY!');

-- 24882: Brutallus 
delete from creature_loot_template where entry=24882;
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 6209, 0, 2, 1, 1, 0, 'Legendary Key');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34181, 6, 2, 1, 1, 0, 'Leggings of Calamity');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34180, 6, 2, 1, 1, 0, 'Felfury Legplates');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34179, 6, 2, 1, 1, 0, 'Heart of the Pit');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34178, 6, 2, 1, 1, 0, 'Collar of the Pit Lord');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34177, 6, 2, 1, 1, 0, 'Clutch of Demise');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34176, 6, 2, 1, 1, 0, 'Reign of Misery');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 12005, 33, 1, -12005, 1, 0, 'Epic Gem - TBC');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 36173, 100, 1, -36173, 2, 0, 'Sunwell Plateau (Boss Loot) - Brutallus (24882) - Tokens (T6 Belt)');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 33117, 0.5, 0, 1, 1, 6012, 'Jack-o\'-Lantern');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 29434, 100, 0, 2, 2, 0, 'Badge of Justice');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34544, -100, 0, 1, 1, 10333, 'Essence of the Immortals');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 34664, 15, 0, 1, 1, 0, 'Sunmote');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (24882, 50501, 0.2, 0, -50501, 1, 0, 'NPC LOOT - Profession (-Design,-Formula,-Pattern,-Plans,-Recipe,-Schematic)(Non-BoP) - NPC Level 64+ Non-Elite/Level 58+ Elite - TBC NPC ONLY!');

-- 25038: Felmyst
delete from creature_loot_template where entry=25038; 
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 6209, 0, 2, 1, 1, 0, 'Legendary Key');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34352, 0, 2, 1, 1, 0, 'Borderland Fortress Grips');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34188, 0, 2, 1, 1, 0, 'Leggings of the Immortal Night');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34186, 0, 2, 1, 1, 0, 'Chain Links of the Tumultuous Storm');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34185, 0, 2, 1, 1, 0, 'Sword Breaker\'s Bulwark');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34184, 0, 2, 1, 1, 0, 'Brooch of the Highborne');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34182, 0, 2, 1, 1, 0, 'Grand Magister\'s Staff of Torrents');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 12005, 33, 1, -12005, 1, 0, 'Epic Gem - TBC');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 36175, 100, 1, -36175, 3, 0, 'Sunwell Plateau (Boss Loot) - Felmyst (25038) - Tokens (T6 Boots)');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 33117, 0.5, 0, 1, 1, 6012, 'Jack-o\'-Lantern');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 29434, 100, 0, 2, 2, 0, 'Badge of Justice');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34544, -100, 0, 1, 1, 10333, 'Essence of the Immortals');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 34664, 15, 0, 1, 1, 0, 'Sunmote');
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `groupid`, `mincountOrRef`, `maxcount`, `condition_id`, `comments`) VALUES (25038, 50501, 0.2, 0, -50501, 1, 0, 'NPC LOOT - Profession (-Design,-Formula,-Pattern,-Plans,-Recipe,-Schematic)(Non-BoP) - NPC Level 64+ Non-Elite/Level 58+ Elite - TBC NPC ONLY!');

-- 25165: Lady Sacrolash 
UPDATE `creature_loot_template` SET `maxcount` = 2 WHERE `entry` = 25165 AND `item` = 36176;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 15 WHERE `entry` = 25166 AND `item` in (35290,35291,35292);

-- 25166: Grand Warlock Alythess 
UPDATE `creature_loot_template` SET `maxcount` = 2 WHERE `entry` = 25166 AND `item` = 36176;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 15 WHERE `entry` = 25166 AND `item` in (35290,35291,35292);

-- 25315: Kil'jaeden 
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 0.5 WHERE `entry` = 25315 AND `item` = 34334;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 50, `maxcount` = 1 WHERE `entry` = 25315 AND `item` = 36195;

-- 25840: Entropius 
UPDATE `creature_loot_template` SET `maxcount` = 1 WHERE `entry` = 25840 AND `item` = 36188;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 10 WHERE `entry` = 25840 AND `item` in (34427,34428,34429,34430);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 15 WHERE `entry` = 25840 AND `item` in (35282,35283,35284);

-- badges rate
-- SELECT a.entry,a.name,b.mincountorref c FROM creature_template a, `creature_loot_template` b WHERE b.`item` = '29434' and b.entry=a.entry order by c desc;
update creature_loot_template set mincountorref=mincountorref*2, maxcount=maxcount*2 where item=29434 and entry in (select entry from creature_template where name like '% (1)');
update creature_loot_template set mincountorref=mincountorref*4, maxcount=maxcount*4 where item=29434 and entry not in (select entry from creature_template where name like '% (1)');

-- Badges of Hero loot from heroic dungeons
insert into creature_loot_template select entry,21065,100,0,1,1,0,'moonwell' from creature_template where entry in (18433,18436,18601,18607,18621,19893,19894,19895,20168,20169,20183,20184,20266,20267,20268,20306,20318,20521,20531,20535,20568,20596,20597,20629,20630,20633,20636,20637,20653,20657,20690,20706,20737,20738,20745,20992,21533,21536,21537,21551,21558,21559,21581,21582,21590,21599,21624,21626,23035,24857,25560,25562,25573);

-- AFTER ALL BELOW

-- all tokens 
update creature_loot_template set maxcount=2 where maxcount>2 and mincountOrRef in (-36004,-36010,-36021,-36023,-36026,-36107,-36108,-36128,-36130,-36132,-36171,-36173,-36175,-40213,-40225,-40300,-40302,-40400);