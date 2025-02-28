UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 20 WHERE `entry` = 3197 AND `item` = 14544;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 20 WHERE `entry` = 3198 AND `item` = 14544;
UPDATE `quest_template` SET `SrcItemId` = 0, `SrcItemCount` = 0, `ReqItemId1` = 14544, `ReqItemCount1` = 1 WHERE `entry` = 5727;