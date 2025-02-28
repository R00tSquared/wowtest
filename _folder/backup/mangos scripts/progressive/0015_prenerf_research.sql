
-- Never comment this, as certain Values are not tuned in further progression files and public doesnt have the expected values

-- All Prenerf Stats Changes must be put into this file
-- Then the related tuning parts have to be updated for the npc that is updated here as tuned values change, when their prenerf base value changes
-- + = researched and set to prenerf hp base value

-- https://wowwiki.fandom.com/wiki/Damage_reduction

-- 																		=====================================
-- 																		===  Pre-Nerf Base Value Research  ==
-- 																		=====================================

-- ============================================================================================================================================================================
-- Open World
-- ============================================================================================================================================================================

-- ============================================================================================================================================================================
-- Isle of Quel'Danas
-- ============================================================================================================================================================================

UPDATE `creature_template` SET `DamageMultiplier` = `DamageMultiplier` / 20 WHERE `entry` IN (
25027, -- Frenzied Ghoul
25028 -- Skeletal Ravager
-- 25030, -- Wrath Enforcer
-- 25031, -- Pit Overlord
-- 25033, -- Eredar Sorcerer
-- 25158, -- Brutallus
-- 25160 -- Madrigosa
);

-- 																		=====================================
-- 																		=====         TBC RAIDS         =====
-- 																		=====================================

-- ============================================================================================================================================================================
-- Karazhan
-- ============================================================================================================================================================================

-- Attumen the Huntsman 15550+ (35H/19D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '24.0', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400', `MinMeleeDmg` = '6792', `MaxMeleeDmg` = '9639', `PowerMultiplier`='10' WHERE `entry` = '15550';
-- Midnight 16151+ (50H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '19.0', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400', `MinMeleeDmg` = '5377', `MaxMeleeDmg` = '7631' WHERE `entry` = '16151';
-- Attumen the Huntsman 16152+ (50H/19.93D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '24.0', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400', `MinMeleeDmg` = '6792', `MaxMeleeDmg` = '9639' WHERE `entry` = '16152';
-- Moroes 15687+ (51H/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '51', `DamageMultiplier` = '17.0', `MinLevelHealth` = '386988', `MaxLevelHealth` = '386988', `PowerMultiplier`='5' WHERE `entry` = '15687';
-- Lady Catriona Von'Indi 19872+ (12H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '12.0', `MinLevelHealth` = '67068', `MaxLevelHealth` = '67068' WHERE `entry` = '19872';
-- Lord Crispin Ference 19873+ (12H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '12.0', `MinLevelHealth` = '83832', `MaxLevelHealth` = '83832', `PowerMultiplier`='8' WHERE `entry` = '19873';
-- Baron Rafe Dreuger 19874+ (12H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '12.0', `MinLevelHealth` = '67068', `MaxLevelHealth` = '67068' WHERE `entry` = '19874';
-- Baroness Dorothea Millstipe 19875+ (12H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '12.0', `MinLevelHealth` = '67068', `MaxLevelHealth` = '67068' WHERE `entry` = '19875';
-- Lord Robin Daris 19876+ (12H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '12.0', `MinLevelHealth` = '83832', `MaxLevelHealth` = '83832', `PowerMultiplier`='8' WHERE `entry` = '19876';
-- Lady Keira Berrybuck 17007+ (12H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '12.0', `MinLevelHealth` = '67068', `MaxLevelHealth` = '67068' WHERE `entry` = '17007';
-- Maiden of Virtue 16457+ (70H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '70', `DamageMultiplier` = '38.0', `MinLevelHealth` = '424900', `MaxLevelHealth` = '424900', `MinMeleeDmg` = '9350', `MaxMeleeDmg` = '13205' WHERE `entry` = '16457';
-- Dorothee 17535+ (25H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `DamageMultiplier` = '15.0', `MinLevelHealth` = '151750', `MaxLevelHealth` = '151750', `MinMeleeDmg` = '4429', `MaxMeleeDmg` = '6255' WHERE `entry` = '17535';
-- Tito 17548+ (5H/4.4D)
UPDATE `creature_template` SET `HealthMultiplier` = '5', `DamageMultiplier` = '7.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '2272', `MaxMeleeDmg` = '3213' WHERE `entry` = '17548';
-- Roar 17546+ (15H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `DamageMultiplier` = '24.0', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700', `MinMeleeDmg` = '4385', `MaxMeleeDmg` = '6200' WHERE `entry` = '17546';
-- Strawman 17543+ (15H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `DamageMultiplier` = '24.0', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700', `MinMeleeDmg` = '7047', `MaxMeleeDmg` = '9965' WHERE `entry` = '17543';
-- Tinhead 17547+ (10.5H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `DamageMultiplier` = '24.0', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700', `MinMeleeDmg` = '7047', `MaxMeleeDmg` = '9965' WHERE `entry` = '17547';
-- The Crone 18168+ (25H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `DamageMultiplier` = '30.0', `MinLevelHealth` = '151750', `MaxLevelHealth` = '151750', `MinMeleeDmg` = '8366', `MaxMeleeDmg` = '11815' WHERE `entry` = '18168';
-- Romulo 17533+ (25H/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `DamageMultiplier` = '22.0', `MinLevelHealth` = '189700', `MaxLevelHealth` = '189700', `MinMeleeDmg` = '7358', `MaxMeleeDmg` = '10443', `PowerMultiplier`='8' WHERE `entry` = '17533';
-- Julianne 17534+ (25H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `DamageMultiplier` = '20.0', `MinLevelHealth` = '151750', `MaxLevelHealth` = '151750', `MinMeleeDmg` = '5905', `MaxMeleeDmg` = '8340' WHERE `entry` = '17534';
-- The Big Bad Wolf 17521+ (50H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '24.0', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400' WHERE `entry` = '17521';
-- The Curator 15691+ (115H/22D) - 698050 hp nerfed version
UPDATE `creature_template` SET `HealthMultiplier` = '189', `DamageMultiplier` = '30.0', `MinLevelHealth` = '1147230', `MaxLevelHealth` = '1147230' WHERE `entry` = '15691';
-- Astral Flare 17096,19781,19782,19783+ (1.33H)
UPDATE `creature_template` SET `HealthMultiplier` = '1.9', `MinLevelHealth` = '13273', `MaxLevelHealth` = '13273' WHERE `entry` IN (17096,19781,19782,19783);
-- Terestian Illhoof 15688+ (100H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `DamageMultiplier` = '26.0', `MinLevelHealth` = '698600', `MaxLevelHealth` = '698600', `MinMeleeDmg` = '6565', `MaxMeleeDmg` = '9282' WHERE `entry` = '15688';
-- Kil'rek 17229+ (8.4H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '26.0', `MinLevelHealth` = '58680', `MaxLevelHealth` = '58680', `MinMeleeDmg` = '3567', `MaxMeleeDmg` = '5174' WHERE `entry` = '17229';
-- Demon Chains 17248+ (1.4H)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972' WHERE `entry` = '17248';
-- Fiendish Imp 17267+ (0.9H/1.1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.1', `DamageMultiplier` = '2.0', `MinLevelHealth` = '5206', `MaxLevelHealth` = '5379', `MinMeleeDmg` = '375', `MaxMeleeDmg` = '569' WHERE `entry` = '17267';
-- Shade of Aran 16524+ (160H/9.3D)
UPDATE `creature_template` SET `HealthMultiplier` = '160.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '849760', `MaxLevelHealth` = '849760', `MinMeleeDmg` = '3325', `MaxMeleeDmg` = '4834' WHERE `entry` = '16524';
-- Conjured Elemental 17167+ (2H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '7.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '1237', `MaxMeleeDmg` = '1749' WHERE `entry` = '17167';
-- Shadow of Aran 18254+ (280H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '400', `DamageMultiplier` = '16.0', `MinLevelHealth` = '1956000', `MaxLevelHealth` = '1956000', `MinMeleeDmg` = '2210', `MaxMeleeDmg` = '3180' WHERE `entry` = '18254';
-- Prince Malchezaar 15690+ (150H/23D)
UPDATE `creature_template` SET `HealthMultiplier` = '150.0', `DamageMultiplier` = '43.0', `MinLevelHealth` = '1138200', `MaxLevelHealth` = '1138200' WHERE `entry` = '15690';
-- Prince Malchezaar's Axes 17650+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier`='1', `DamageMultiplier` = '3.0', `MinLevelHealth` = '7588', `MaxLevelHealth` = '7588' WHERE `entry` = '17650';
-- Netherspite 15689+ (200H/24.75D)
UPDATE `creature_template` SET `HealthMultiplier` = '200', `MinLevelHealth` = '1117800', `MaxLevelHealth` = '1117800', `DamageMultiplier` = '29.0' WHERE `entry` = '15689';
-- Nightbane 17225+ (175H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '175', `MinLevelHealth` = '1327900', `MaxLevelHealth` = '1327900', `DamageMultiplier` = '38.0', `PowerMultiplier`='20' WHERE `entry` = '17225';
-- Restless Skeleton 17261+ (2.5H/7D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `MinLevelHealth` = '16355', `MaxLevelHealth` = '17465', `DamageMultiplier` = '7.0' WHERE `entry` = '17261';
-- Hyakiss the Lurker 16179+ (35H/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `DamageMultiplier` = '20.0', `MinLevelHealth` = '265580', `MaxLevelHealth` = '265580', `MinMeleeDmg` = '5660', `MaxMeleeDmg` = '8033' WHERE `entry` = '16179';
-- Shadikith the Glider 16180+ (35H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `DamageMultiplier` = '29.0', `MinLevelHealth` = '265580', `MaxLevelHealth` = '265580', `MinMeleeDmg` = '5745', `MaxMeleeDmg` = '8153' WHERE `entry` = '16180';
-- Rokad the Ravager 16181+ (30H/34D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `DamageMultiplier` = '40.0', `MinLevelHealth` = '227640', `MaxLevelHealth` = '227640', `MinMeleeDmg` = '4245', `MaxMeleeDmg` = '6025' WHERE `entry` = '16181';

-- ============================
-- Trash
-- ============================

-- Spectral Charger 15547+ (9H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `DamageMultiplier` = '19.0', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `MinMeleeDmg` = '4878', `MaxMeleeDmg` = '6898' WHERE `entry` = '15547';
-- Spectral Stallion 15548+ (6.3H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `DamageMultiplier` = '8.5', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `MinMeleeDmg` = '2182', `MaxMeleeDmg` = '3086' WHERE `entry` = '15548';
-- Spectral Stable Hand 15551+ (6H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '11.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '2695', `MaxMeleeDmg` = '3807' WHERE `entry` = '15551';
-- Coldmist Stalker 16170+ (3H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '3', `DamageMultiplier` = '10.0', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `MinMeleeDmg` = '1262', `MaxMeleeDmg` = '1785' WHERE `entry` = '16170';
-- Coldmist Widow 16171+ (7H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '7', `DamageMultiplier` = '16.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `MinMeleeDmg` = '4107', `MaxMeleeDmg` = '5809' WHERE `entry` = '16171';
-- Shadowbat 16173+ (3H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '3', `DamageMultiplier` = '10.0', `MinLevelHealth` = '20283', `MaxLevelHealth` = '20958', `MinMeleeDmg` = '2416', `MaxMeleeDmg` = '3570' WHERE `entry` = '16173';
-- Greater Shadowbat 16174+ (8H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '20.0', `MinLevelHealth` = '57448', `MaxLevelHealth` = '57448', `MinMeleeDmg` = '5134', `MaxMeleeDmg` = '7261' WHERE `entry` = '16174';
-- Vampiric Shadowbat 16175+ (6H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '4107', `MaxMeleeDmg` = '5809' WHERE `entry` = '16175';
-- Shadowbeast 16176+ (6H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '40566', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '3383', `MaxMeleeDmg` = '4998' WHERE `entry` = '16176';
-- Dreadbeast 16177+ (7H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '7', `DamageMultiplier` = '20.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `MinMeleeDmg` = '5134', `MaxMeleeDmg` = '7261' WHERE `entry` = '16177';
-- Phase Hound 16178+ (3.5H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '4', `DamageMultiplier` = '12.0', `MinLevelHealth` = '27944', `MaxLevelHealth` = '27944', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4284' WHERE `entry` = '16178';
-- Spectral Apprentice 16389+ (6H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '3535', `MaxMeleeDmg` = '4998' WHERE `entry` = '16389';
-- Phantom Attendant 16406+ (4.2H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '34464', `MaxLevelHealth` = '34464', `MinMeleeDmg` = '3335', `MaxMeleeDmg` = '4713' WHERE `entry` = '16406';
-- Spectral Servant 16407+ (6H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '40566', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '3383', `MaxMeleeDmg` = '4998' WHERE `entry` = '16407';
-- Phantom Valet 16408+ (9H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `DamageMultiplier` = '27.0', `MinLevelHealth` = '66420', `MaxLevelHealth` = '66420', `MinMeleeDmg` = '7047', `MaxMeleeDmg` = '9965' WHERE `entry` = '16408';
-- Phantom Guest 16409+ (1.4H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `DamageMultiplier` = '4.0', `MinLevelHealth` = '11178', `MaxLevelHealth` = '11178', `MinMeleeDmg` = '1640', `MaxMeleeDmg` = '2317' WHERE `entry` = '16409';
-- Spectral Retainer 16410+ (8H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '24.0', `MinLevelHealth` = '59040', `MaxLevelHealth` = '59040', `MinMeleeDmg` = '6264', `MaxMeleeDmg` = '8858' WHERE `entry` = '16410';
-- Spectral Chef 16411+ (6H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '23.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '5904', `MaxMeleeDmg` = '8351' WHERE `entry` = '16411';
-- Ghostly Baker 16412+ (4.2H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '16412';
-- Ghostly Steward 16414+ (5.6H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '14.0', `MinLevelHealth` = '57448', `MaxLevelHealth` = '57448', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '16414';
-- Skeletal Waiter 16415+ (6H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '17.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '4364', `MaxMeleeDmg` = '6172' WHERE `entry` = '16415';
-- Spectral Sentry 16424+ (6H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '17.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '4364', `MaxMeleeDmg` = '6172' WHERE `entry` = '16424';
-- Phantom Guardsman 16425+ (4.2H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '16425';
-- Wanton Hostess 16459+ (8H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '17.0', `MinLevelHealth` = '57448', `MaxLevelHealth` = '57448', `MinMeleeDmg` = '4364', `MaxMeleeDmg` = '6172' WHERE `entry` = '16459';
-- Night Mistress 16460+ (8H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '17.0', `MinLevelHealth` = '45952', `MaxLevelHealth` = '45952', `MinMeleeDmg` = '4050', `MaxMeleeDmg` = '5723' WHERE `entry` = '16460';
-- Concubine 16461+ (8H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '17.0', `MinLevelHealth` = '57448', `MaxLevelHealth` = '57448', `MinMeleeDmg` = '4364', `MaxMeleeDmg` = '6172' WHERE `entry` = '16461';
-- Spectral Patron 16468+ (2H/4.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `DamageMultiplier` = '8.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972', `MinMeleeDmg` = '2020', `MaxMeleeDmg` = '2856' WHERE `entry` = '16468';
-- Ghostly Philanthropist 16470+ (5.6H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '15.0', `MinLevelHealth` = '47224', `MaxLevelHealth` = '47224', `MinMeleeDmg` = '3630', `MaxMeleeDmg` = '5128' WHERE `entry` = '16470';
-- Skeletal Usher 16471+ (12H/22D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '27.0', `MinLevelHealth` = '70836', `MaxLevelHealth` = '70836', `MinMeleeDmg` = '6534', `MaxMeleeDmg` = '9230' WHERE `entry` = '16471';
-- Phantom Stagehand 16472+ (6.3H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `DamageMultiplier` = '31.0', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `MinMeleeDmg` = '7958', `MaxMeleeDmg` = '11255' WHERE `entry` = '16472';
-- Spectral Performer 16473+ (8H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '24.0', `MinLevelHealth` = '57448', `MaxLevelHealth` = '57448', `MinMeleeDmg` = '6161', `MaxMeleeDmg` = '8714', `PowerMultiplier`='5' WHERE `entry` = '16473';
-- Ghastly Haunt 16481+ (10H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `DamageMultiplier` = '35.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `MinMeleeDmg` = '9135', `MaxMeleeDmg` = '12917' WHERE `entry` = '16481';
-- Trapped Soul 16482+ (7H/23D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `DamageMultiplier` = '26.0', `MinLevelHealth` = '59030', `MaxLevelHealth` = '59030', `MinMeleeDmg` = '6292', `MaxMeleeDmg` = '8888' WHERE `entry` = '16482';
-- Arcane Watchman 16485+ (12H/22D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '25.0', `MinLevelHealth` = '88560', `MaxLevelHealth` = '88560', `MinMeleeDmg` = '6525', `MaxMeleeDmg` = '9227' WHERE `entry` = '16485';
-- Arcane Anomaly 16488+ (7M/16D) - Mana
UPDATE `creature_template` SET `Powermultiplier` = '7', `DamageMultiplier` = '18.0', `MinLevelMana` = '51324', `MaxLevelMana` = '51324', `MinMeleeDmg` = '3679', `MaxMeleeDmg` = '5342' WHERE `entry` = '16488';
-- Chaotic Sentience 16489+ (8H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '18.0', `MinLevelHealth` = '55888', `MaxLevelHealth` = '57448', `MinMeleeDmg` = '2727', `MaxMeleeDmg` = '3921' WHERE `entry` = '16489';
-- Mana Feeder 16491+ (2H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `DamageMultiplier` = '8.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972', `MinMeleeDmg` = '1010', `MaxMeleeDmg` = '1428' WHERE `entry` = '16491';
-- Syphoner 16492+ (2H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `DamageMultiplier` = '8.0', `MinLevelHealth` = '11178', `MaxLevelHealth` = '11178', `MinMeleeDmg` = '1875', `MaxMeleeDmg` = '2649' WHERE `entry` = '16492';
-- Arcane Protector 16504+ (16H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '16', `DamageMultiplier` = '36.0', `MinLevelHealth` = '118080', `MaxLevelHealth` = '118080', `MinMeleeDmg` = '9396', `MaxMeleeDmg` = '13287' WHERE `entry` = '16504';
-- Spell Shade 16525+ (6H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '15.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `MinMeleeDmg` = '3851', `MaxMeleeDmg` = '5446' WHERE `entry` = '16525';
-- Sorcerous Shade 16526+ (8H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '15.0', `MinLevelHealth` = '47224', `MaxLevelHealth` = '47224', `MinMeleeDmg` = '3630', `MaxMeleeDmg` = '5128' WHERE `entry` = '16526';
-- Magical Horror 16529+ (8H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '14.0', `MinLevelHealth` = '45952', `MaxLevelHealth` = '45952', `MinMeleeDmg` = '3335', `MaxMeleeDmg` = '4713' WHERE `entry` = '16529';
-- Mana Warp 16530+ (6H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '8.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '2020', `MaxMeleeDmg` = '2856' WHERE `entry` = '16530';
-- Homunculus 16539+ (3H/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '3', `DamageMultiplier` = '5.0', `MinLevelHealth` = '16767', `MaxLevelHealth` = '16767', `MinMeleeDmg` = '1172', `MaxMeleeDmg` = '1655' WHERE `entry` = '16539';
-- Shadow Pillager 16540+ (6H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '10.0', `MinLevelHealth` = '35418', `MaxLevelHealth` = '35418', `MinMeleeDmg` = '2420', `MaxMeleeDmg` = '3419' WHERE `entry` = '16540';
-- Ethereal Thief 16544+ (8H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `DamageMultiplier` = '27.0', `MinLevelHealth` = '59040', `MaxLevelHealth` = '59040', `MinMeleeDmg` = '7047', `MaxMeleeDmg` = '9965' WHERE `entry` = '16544';
-- Ethereal Spellfilcher 16545+ (10H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `DamageMultiplier` = '28.0', `MinLevelHealth` = '59030', `MaxLevelHealth` = '59030', `MinMeleeDmg` = '6776', `MaxMeleeDmg` = '9572' WHERE `entry` = '16545';
-- Fleshbeast 16595+ (7H/22D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `DamageMultiplier` = '27.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `MinMeleeDmg` = '5220', `MaxMeleeDmg` = '7381' WHERE `entry` = '16595';
-- Greater Fleshbeast 16596+ (16H/36D)
UPDATE `creature_template` SET `HealthMultiplier` = '16', `DamageMultiplier` = '41.0', `MinLevelHealth` = '118080', `MaxLevelHealth` = '118080', `MinMeleeDmg` = '8352', `MaxMeleeDmg` = '11810' WHERE `entry` = '16596';
-- Phantom Hound 17067+ (1.4H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `DamageMultiplier` = '2.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972', `MinMeleeDmg` = '353', `MaxMeleeDmg` = '500' WHERE `entry` = '17067';

-- ============================================================================================================================================================================
-- Gruul's Lair
-- ============================================================================================================================================================================

-- High King Maulgar 18831+ (100H/39.984D)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `DamageMultiplier` = '40.0', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `MinMeleeDmg` = '13018', `MaxMeleeDmg` = '18476' WHERE `entry` = '18831';
-- Krosh Firehand 18832+ (50H/16.67D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '30.0', `MinLevelHealth` = '303500', `MaxLevelHealth` = '303500', `MinMeleeDmg` = '7381', `MaxMeleeDmg` = '10425' WHERE `entry` = '18832';
-- Olm the Summoner 18834+ (50H/16.67D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '32.0', `MinLevelHealth` = '303500', `MaxLevelHealth` = '303500', `MinMeleeDmg` = '7873', `MaxMeleeDmg` = '11120' WHERE `entry` = '18834';
-- Kiggler the Crazed 18835+ (50H/27.777D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '50.0', `MinLevelHealth` = '303500', `MaxLevelHealth` = '303500', `MinMeleeDmg` = '12302', `MaxMeleeDmg` = '17375' WHERE `entry` = '18835';
-- Blindeye the Seer 18836+ (50H/7D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `DamageMultiplier` = '25.0', `MinLevelHealth` = '303500', `MaxLevelHealth` = '303500', `MinMeleeDmg` = '6151', `MaxMeleeDmg` = '8687' WHERE `entry` = '18836';
-- Wild Fel Stalker 18847+ (4H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `DamageMultiplier` = '12.0', `MinLevelHealth` = '60704', `MaxLevelHealth` = '60704', `MinMeleeDmg` = '2377', `MaxMeleeDmg` = '3374' WHERE `entry` = '18847';
-- Gruul the Dragonkiller 19044+ (450H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '633', `DamageMultiplier` = '30.0', `MinLevelHealth` = '4803204', `MaxLevelHealth` = '4803204', `MinMeleeDmg` = '8490', `MaxMeleeDmg` = '12049' WHERE `entry` = '19044';

-- ============================
-- Trash
-- ============================

-- Lair Brute 19389+ (40H/30.047D)
UPDATE `creature_template` SET `HealthMultiplier` = '40.0', `DamageMultiplier` = '35.0', `MinLevelHealth` = '295200', `MaxLevelHealth` = '295200', `MinMeleeDmg` = '9135', `MaxMeleeDmg` = '12917' WHERE `entry` = '19389';
-- Gronn-Priest 21350+ (40H/26.053D)
UPDATE `creature_template` SET `HealthMultiplier` = '42.0', `DamageMultiplier` = '30.0', `MinLevelHealth` = '247926', `MaxLevelHealth` = '247926', `MinMeleeDmg` = '7260', `MaxMeleeDmg` = '10257' WHERE `entry` = '21350';

-- ============================================================================================================================================================================
-- Magtheridon's Lair
-- ============================================================================================================================================================================

-- Magtheridon 17257+ (635H/50D)
UPDATE `creature_template` SET `HealthMultiplier` = '635.0', `MinLevelHealth` = '4818380', `MaxLevelHealth` = '4818380', `DamageMultiplier` = '50.0' WHERE `entry` = '17257';
-- Hellfire Channeler 17256+ (40H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200' WHERE `entry` = '17256'; -- pre 2.1 (https://www.youtube.com/watch?v=A5Ma8G8ZScc)

-- ============================
-- Trash
-- ============================

-- Hellfire Warder 18829+ (32H/22D)
UPDATE `creature_template` SET `HealthMultiplier` = '50.0', `DamageMultiplier` = '33.0', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150', `MinMeleeDmg` = '7986', `MaxMeleeDmg` = '11281' WHERE `entry` = '18829';

-- ============================================================================================================================================================================
-- World Bosses
-- ============================================================================================================================================================================

-- Lord Kazzak 12397 (110H/50D)
UPDATE `creature_template` SET `UnitFlags` = 32832, `ExtraFlags` = 266240, `SpeedWalk` = 1, `SpeedRun` = 2.14286, `Detection` = 35, `HealthMultiplier` = 330, `PowerMultiplier`='2', `MinLevelHealth` = '1099230', `MaxLevelHealth` = '1099230', `DamageMultiplier` = 14, `MeleeBaseAttackTime` = 2000, `MechanicImmuneMask` = 650854239 WHERE `entry` = 12397;
UPDATE `creature_model_info` SET `bounding_radius` = 9, `combat_reach` = 13 WHERE `modelid` = 12449;
-- Doomwalker 17711+ (300H/33D)
UPDATE `creature_template` SET `HealthMultiplier` = '340.0', `DamageMultiplier` = '36.0', `MinLevelHealth` = '2579920', `MaxLevelHealth` = '2579920', `MinMeleeDmg` = '15282', `MaxMeleeDmg` = '21689' WHERE `entry` = '17711';
-- Doom Lord Kazzak 18728+ (160H/90D)
UPDATE `creature_template` SET `HealthMultiplier` = '172.0', `MinLevelHealth` = '1305136', `MaxLevelHealth` = '1305136' WHERE `entry` = '18728';

-- ============================================================================================================================================================================
-- Coilfang Reservoir: Serpentshrine Cavern
-- ============================================================================================================================================================================

-- Hydross the Unstable 21216 (450H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '500', `MinLevelHealth` = '3794000', `MaxLevelHealth` = '3794000' WHERE `entry` = '21216';
-- Tainted Spawn of Hydross 22036 (4.2H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629' WHERE `entry` = '22036';
-- Pure Spawn of Hydross 22035 (6H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629' WHERE `entry` = '22035';
-- The Lurker Below 21217 (500H/42D)
UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '5311600', `MaxLevelHealth` = '5311600', `DamageMultiplier` = '42' WHERE `entry` = '21217';
-- Coilfang Ambusher 21865 (5H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '55890', `MaxLevelHealth` = '55890', `DamageMultiplier` = '1' WHERE `entry` = '21865';
-- Coilfang Guardian 21873 (10H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '69860', `MaxLevelHealth` = '69860', `DamageMultiplier` = '30' WHERE `entry` = '21873';
-- Leotheras the Blind 21215 (500H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '600', `MinLevelHealth` = '4552800', `MaxLevelHealth` = '4552800', `DamageMultiplier` = '28' WHERE `entry` = '21215';
-- Greyheart Spellbinder 21806 (20H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '344640', `MaxLevelHealth` = '344640', `DamageMultiplier` = '24' WHERE `entry` = '21806';
-- Inner Demon 21857+ (1.488H/5.2D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.75', `MinLevelHealth` = '12226', `MaxLevelHealth` = '12226', `DamageMultiplier` = '5.2' WHERE `entry` = '21857';
-- Fathom-Lord Karathress 21214 (300H/36D)
UPDATE `creature_template` SET `HealthMultiplier` = '300', `MinLevelHealth` = '1821000', `MaxLevelHealth` = '1821000', `DamageMultiplier` = '36' WHERE `entry` = '21214';
-- Fathom-Guard Caribdis 21964 (150H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '150', `MinLevelHealth` = '861600', `MaxLevelHealth` = '861600', `DamageMultiplier` = '25' WHERE `entry` = '21964';
-- Fathom-Guard Tidalvess 21965 (150H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '150', `MinLevelHealth` = '861600', `MaxLevelHealth` = '861600', `DamageMultiplier` = '42' WHERE `entry` = '21965';
-- Fathom-Guard Sharkkis 21966 (150H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '150', `MinLevelHealth` = '861600', `MaxLevelHealth` = '861600', `DamageMultiplier` = '25' WHERE `entry` = '21966';
-- Fathom Lurker 22119 (25H/26D) - unitclass wrong?
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '11' WHERE `entry` = '22119';
-- Fathom Sporebat 22120 (25H/8) - unitclass wrong?
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '11' WHERE `entry` = '22120';
-- Morogrim Tidewalker 21213 (750H/56D)
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '5691000', `MaxLevelHealth` = '5691000', `DamageMultiplier` = '56' WHERE `entry` = '21213';
-- Tidewalker Lurker 21920+ (1.75H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '71810', `MaxLevelHealth` = '71810', `DamageMultiplier` = '6' WHERE `entry` = '21920';
-- Lady Vashj 21212+ (800H/45D)
UPDATE `creature_template` SET `HealthMultiplier` = '1000', `MinLevelHealth` = '6070000', `MaxLevelHealth` = '6070000', `DamageMultiplier` = '45' WHERE `entry` = '21212';
-- Tainted Elemental 22009+ (0.7H/0.75D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '0.75' WHERE `entry` = '22009';
-- Enchanted Elemental 21958+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '1' WHERE `entry` = '21958';
-- Coilfang Elite 22055+ (25H/31D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '279400', `MaxLevelHealth` = '279400', `DamageMultiplier` = '31' WHERE `entry` = '22055';
-- Toxic Spore Bat 22140+ (1H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `DamageMultiplier` = '1' WHERE `entry` = '22140';
-- Coilfang Strider 22056+ (25H/60D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '419160', `MaxLevelHealth` = '419160', `DamageMultiplier` = '60' WHERE `entry` = '22056';

-- ============================
-- Trash
-- ============================

-- Tainted Water Elemental 21253 (1.05H/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `MinLevelHealth` = '10772', `MaxLevelHealth` = '10772', `DamageMultiplier` = '3' WHERE `entry` = '21253';
-- Purified Water Elemental 21260 (1.5H/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `MinLevelHealth` = '10772', `MaxLevelHealth` = '10772', `DamageMultiplier` = '3' WHERE `entry` = '21260';
-- Underbog Colossus (21251) (75H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '120',`MinLevelHealth` = '885600', `MaxLevelHealth` = '885600', `DamageMultiplier` = '55' WHERE `entry` = '21251';
-- Colossus Rager 22352 (4H/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '125748', `MaxLevelHealth` = '125748', `DamageMultiplier` = '6' WHERE `entry` = '22352';
-- Coilfang Beast-Tamer (21221) (35H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '430860', `MaxLevelHealth` = '430860', `DamageMultiplier` = '40' WHERE `entry` = '21221';
-- Tidewalker Depth-Seer 21224 (17.5H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '20' WHERE `entry` = '21224';
-- Tidewalker Warrior 21225 (17.5H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '20' WHERE `entry` = '21225';
-- Tidewalker Shaman 21226 (17.5H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '16' WHERE `entry` = '21226';
-- Tidewalker Harpooner 21227 (17.5H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '20' WHERE `entry` = '21227';
-- Tidewalker Hydromancer 21228 (17.5H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '16' WHERE `entry` = '21228';
-- Greyheart Tidecaller 21229 (25H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '20' WHERE `entry` = '21229';
-- Coilfang Serpentguard (21298) (25H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '35' WHERE `entry` = '21298';
-- Coilfang Fathom-Witch 21299 (25H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '16' WHERE `entry` = '21299';
-- Greyheart Nether-Mage 21230 (25H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '16' WHERE `entry` = '21230';
-- Greyheart Shield-Bearer 21231 (25H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '25' WHERE `entry` = '21231';
-- Greyheart Skulker 21232 (25H/35D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '35' WHERE `entry` = '21232';
-- Coilfang Hate-Screamer 21339 (25H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '16' WHERE `entry` = '21339';
-- Serpentshrine Lurker 21863 (10.5H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '279440', `MaxLevelHealth` = '279440', `DamageMultiplier` = '24' WHERE `entry` = '21863';
-- Greyheart Technician 21263 (2H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '4' WHERE `entry` = '21263';
-- Coilfang Priestess (21220) (25H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '24' WHERE `entry` = '21220';
-- Coilfang Shatterer (21301) (25H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '40' WHERE `entry` = '21301';
-- Vashj'ir Honor Guard (21218) (24.5H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '430860', `MaxLevelHealth` = '430860', `DamageMultiplier` = '40' WHERE `entry` = '21218';
-- Serpentshrine Sporebat 21246 (10.5H/15D) - 15H nerfed
UPDATE `creature_template` SET `HealthMultiplier` = '24', `MinLevelHealth` = '172344', `MaxLevelHealth` = '172344', `DamageMultiplier` = '15' WHERE `entry` = '21246';
-- Colossus Lurker 22347 (20H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '215430', `MaxLevelHealth` = '215430', `DamageMultiplier` = '40' WHERE `entry` = '22347';
-- Coilfang Frenzy 21508 (20H/7.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '81000', `MaxLevelHealth` = '81000', `DamageMultiplier` = '10' WHERE `entry` = '21508';
-- Serpentshrine Tidecaller 22238 (15H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '21', `MinLevelHealth` = '120624', `MaxLevelHealth` = '120624', `DamageMultiplier` = '18' WHERE `entry` = '22238';

-- ============================================================================================================================================================================
-- Tempest Keep: The Eye
-- ============================================================================================================================================================================

-- Al'ar 19514+ (280H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '450', `MinLevelHealth` = '3414600', `MaxLevelHealth` = '3414600', `DamageMultiplier` = '30' WHERE `entry` = '19514';
-- Ember of Al'ar 19551 (7H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '139720', `MaxLevelHealth` = '139720', `DamageMultiplier` = '10' WHERE `entry` = '19551';
-- Void Reaver 19516 (750H/36D)
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '4552500', `MaxLevelHealth` = '4552500', `DamageMultiplier` = '36' WHERE `entry` = '19516';
-- High Astromancer Solarian 18805 (500H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '4552500', `MaxLevelHealth` = '4552500', `DamageMultiplier` = '25' WHERE `entry` = '18805';
-- Solarium Agent 18925 (2.8H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '4', `MinLevelHealth` = '27044', `MaxLevelHealth` = '27044', `DamageMultiplier` = '4' WHERE `entry` = '18925';
-- Solarium Priest 18806 (10.5H/0.8D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '77490', `MaxLevelHealth` = '77490', `DamageMultiplier` = '0.8' WHERE `entry` = '18806';
-- Kael'thas Sunstrider 19622+ (420H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '1000', `MinLevelHealth` = '6070000', `MaxLevelHealth` = '6070000', `DamageMultiplier` = '40' WHERE `entry` = '19622';
-- Lord Sanguinar 20060+ (36H/30D) (7588)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '30' WHERE `entry` = '20060';
-- Grand Astromancer Capernian 20062+ (36H/11D) (5311)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `MinLevelHealth` = '531100', `MaxLevelHealth` = '531100', `DamageMultiplier` = '11' WHERE `entry` = '20062';
-- Master Engineer Telonicus 20063+ (25.2H/20D) (7588)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '15' WHERE `entry` = '20063';
-- Thaladred the Darkener 20064+ (25.2H/30D) (7588)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '30' WHERE `entry` = '20064';
-- Netherstrand Longbow 21268+ (18.9H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '209580', `MaxLevelHealth` = '209580', `DamageMultiplier` = '15' WHERE `entry` = '21268';
-- Devastation 21269+ (31.5H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '244510', `MaxLevelHealth` = '244510', `DamageMultiplier` = '30' WHERE `entry` = '21269';
-- Cosmic Infuser 21270+ (36H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '279440', `MaxLevelHealth` = '279440', `DamageMultiplier` = '15' WHERE `entry` = '21270';
-- Infinity Blades 21271+ (18.9H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '209580', `MaxLevelHealth` = '209580', `DamageMultiplier` = '30' WHERE `entry` = '21271';
-- Warp Slicer 21272+ (36H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '279440', `MaxLevelHealth` = '279440', `DamageMultiplier` = '15' WHERE `entry` = '21272';
-- Phaseshift Bulwark 21273+ (28.35H/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '314370', `MaxLevelHealth` = '314370', `DamageMultiplier` = '13' WHERE `entry` = '21273';
-- Staff of Disintegration 21274+ (15.75H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '174650', `MaxLevelHealth` = '174650', `DamageMultiplier` = '15' WHERE `entry` = '21274';
-- Phoenix 21362 (25H/13D)
UPDATE `creature_template` SET `MinLevel` = 73, `MaxLevel` = 73, `HealthMultiplier` = '100', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '13' WHERE `entry` = '21362';
-- Phoenix Egg 21364+ (7H)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '174650', `MaxLevelHealth` = '174650' WHERE `entry` = '21364';

-- ============================
-- Trash
-- ============================

-- Bloodwarder Legionnaire 20031 (25H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '28' WHERE `entry` = '20031';
-- Bloodwarder Vindicator 20032 (40H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '80', `MinLevelHealth` = '459520', `MaxLevelHealth` = '459520', `DamageMultiplier` = '32' WHERE `entry` = '20032';
-- Astromancer 20033 (25H/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '14' WHERE `entry` = '20033';
-- Star Scryer 20034 (25H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '16' WHERE `entry` = '20034';
-- Bloodwarder Marshal 20035 (40H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '80', `MinLevelHealth` = '590400', `MaxLevelHealth` = '590400', `DamageMultiplier` = '32' WHERE `entry` = '20035';
-- Bloodwarder Squire 20036 (25H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '24' WHERE `entry` = '20036';
-- Tempest Falconer 20037 (17.5H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760', `DamageMultiplier` = '10' WHERE `entry` = '20037';
-- Phoenix-Hawk Hatchling 20038 (7H/14D) (nerfed 10.30644153H 72k)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '104790', `MaxLevelHealth` = '104790', `DamageMultiplier` = '14' WHERE `entry` = '20038';
-- Phoenix-Hawk 20039 (10H/16D) - completely wrong (nerfed 75H - 553500) - (prenerf 738000)
UPDATE `creature_template` SET `MinLevel` = 72, `MaxLevel` = 72, `HealthMultiplier` = '100', `MinLevelHealth` = '738000', `MaxLevelHealth` = '738000', `DamageMultiplier` = '40' WHERE `entry` = '20039';
-- Crystalcore Devastator 20040 (75H/32D) - (nerfed 75H - 553500)
UPDATE `creature_template` SET `HealthMultiplier` = '130', `MinLevelHealth` = '959400', `MaxLevelHealth` = '959400', `DamageMultiplier` = '32' WHERE `entry` = '20040';
-- Crystalcore Sentinel 20041 (40H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '70', `MinLevelHealth` = '516600', `MaxLevelHealth` = '516600', `DamageMultiplier` = '28' WHERE `entry` = '20041';
-- Tempest-Smith 20042 (17.5H/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '28' WHERE `entry` = '20042';
-- Apprentice Star Scryer 20043 (2.8H/10D) - (nerfed 4.473178927 - 25000)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '67068', `MaxLevelHealth` = '67068', `DamageMultiplier` = '10' WHERE `entry` = '20043';
-- Novice Astromancer 20044 (2.8H/10D) - (nerfed 4.65216893 - 26000)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '67068', `MaxLevelHealth` = '67068', `DamageMultiplier` = '10' WHERE `entry` = '20044';
-- Nether Scryer 20045 (40H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '80', `MinLevelHealth` = '472240', `MaxLevelHealth` = '472240', `DamageMultiplier` = '20' WHERE `entry` = '20045';
-- Astromancer Lord 20046 (40H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '80', `MinLevelHealth` = '472240', `MaxLevelHealth` = '472240', `DamageMultiplier` = '20' WHERE `entry` = '20046';
-- Crimson Hand Battle Mage 20047 (17.5H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '48', `MinLevelHealth` = '275712', `MaxLevelHealth` = '275712', `DamageMultiplier` = '16' WHERE `entry` = '20047';
-- Crimson Hand Centurion 20048 (17.5H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '48', `MinLevelHealth` = '344688', `MaxLevelHealth` = '344688', `DamageMultiplier` = '15' WHERE `entry` = '20048';
-- Crimson Hand Blood Knight 20049 (28H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '70', `MinLevelHealth` = '402080', `MaxLevelHealth` = '402080', `DamageMultiplier` = '16' WHERE `entry` = '20049';
-- Crimson Hand Inquisitor 20050 (28H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '80', `MinLevelHealth` = '472240', `MaxLevelHealth` = '472240', `DamageMultiplier` = '15' WHERE `entry` = '20050';
-- Crystalcore Mechanic 20052 (25H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '20' WHERE `entry` = '20052';

-- ============================================================================================================================================================================
-- Caverns of Time: Hyjal Summit
-- ============================================================================================================================================================================

-- Rage Winterchill 17767 (700H/45D)
UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '4249000', `MaxLevelHealth` = '4249000', `DamageMultiplier` = '45' WHERE `entry` = '17767';
-- Anetheron 17808 (700H/50D)
UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '4249000', `MaxLevelHealth` = '4249000', `DamageMultiplier` = '50' WHERE `entry` = '17808';
-- Towering Infernal 17818 (30H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '221400', `MaxLevelHealth` = '221400', `DamageMultiplier` = '20' WHERE `entry` = '17818';
-- Kaz'rogal 17888 (490H/50D)
UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '4249000', `MaxLevelHealth` = '4249000', `DamageMultiplier` = '50' WHERE `entry` = '17888';
-- Azgalor 17842 (700H/85D)
UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '4249000', `MaxLevelHealth` = '4249000', `DamageMultiplier` = '85' WHERE `entry` = '17842';
-- Lesser Doomguard 17864 (35H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '206605', `MaxLevelHealth` = '206605', `DamageMultiplier` = '15' WHERE `entry` = '17864';
-- Archimonde 17968 (750H/110D)
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '4552500', `MaxLevelHealth` = '4552500', `DamageMultiplier` = '110' WHERE `entry` = '17968';

-- ============================
-- Trash
-- ============================

-- Ghoul 17895+ (20H/10D) 6986
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '139720', `MaxLevelHealth` = '139720', `DamageMultiplier` = '10' WHERE `entry` = '17895';
-- Crypt Fiend 17897+ (25H/14.27D) 6986
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '174650', `MaxLevelHealth` = '174650', `DamageMultiplier` = '14.27' WHERE `entry` = '17897';
-- Abomination 17898+ (25H/17D) 7181
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '179525', `MaxLevelHealth` = '179525', `DamageMultiplier` = '17' WHERE `entry` = '17898';
-- Shadowy Necromancer 17899+ (15.4H/17D) 5589
UPDATE `creature_template` SET `HealthMultiplier` = '22', `MinLevelHealth` = '122958', `MaxLevelHealth` = '122958', `DamageMultiplier` = '17' WHERE `entry` = '17899';
-- Skeleton Invader 17902+ (5H/4.4D) 6761
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '33805', `MaxLevelHealth` = '34930', `DamageMultiplier` = '4.4' WHERE `entry` = '17902';
-- Skeleton Mage 17903+ (5H/4.5D) 4733
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '23665', `MaxLevelHealth` = '24450', `DamageMultiplier` = '4.5' WHERE `entry` = '17903';
-- Banshee 17905+ (15H/9D) 5589
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '83835', `MaxLevelHealth` = '83835', `DamageMultiplier` = '9' WHERE `entry` = '17905';
-- Gargoyle 17906+ (18H/13D) 6986
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '125748', `MaxLevelHealth` = '125748', `DamageMultiplier` = '13' WHERE `entry` = '17906';
-- Frost Wyrm 17907+ (45H/20D) 7380
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '332100', `MaxLevelHealth` = '332100', `DamageMultiplier` = '20' WHERE `entry` = '17907';
-- Giant Infernal 17908+ (18H/8D) 7181
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '129258', `MaxLevelHealth` = '129258', `DamageMultiplier` = '8' WHERE `entry` = '17908';
-- Fel Stalker 17916+ (15H/10D) 6986
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '104790', `MaxLevelHealth` = '104790', `DamageMultiplier` = '10' WHERE `entry` = '17916';

-- Alliance Footman 17919 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17919'; -- 16
-- Alliance Knight 17920 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17920'; -- 16
-- Alliance Rifleman 17921 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17921'; -- 16
-- Alliance Sorceress 17922 
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '17922'; -- 20
-- Alliance Priest 17928 
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '17928'; -- 20
-- Alliance Peasant 17931 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17931'; -- 15
-- Lady Jaina Proudmoore 17772 
UPDATE `creature_template` SET `HealthMultiplier` = '75', `MinLevelHealth` = '455250', `MaxLevelHealth` = '455250', `DamageMultiplier` = '7.5' WHERE `entry` = '17772'; -- 30

-- Horde Grunt 17932 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17932'; -- 16
-- Tauren Warrior 17933 
UPDATE `creature_template` SET `DamageMultiplier` = '3' WHERE `entry` = '17933'; -- 10
-- Horde Headhunter 17934 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17934'; -- 16
-- Horde Witch Doctor 17935 
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '17935'; -- 17
-- Horde Shaman 17936 
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '17936'; -- 17
-- Horde Peon 17937 
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '17937'; -- 16
-- Dire Wolf 17854 
-- UPDATE `creature_template` SET `DamageMultiplier` = 'XXX' WHERE `entry` = '17854'; -- 1.9
-- Thrall 17852 
UPDATE `creature_template` SET `HealthMultiplier` = '75', `MinLevelHealth` = '455250', `MaxLevelHealth` = '455250', `DamageMultiplier` = '3.5' WHERE `entry` = '17852'; -- 7

-- Druid of the Talon 3794 
UPDATE `creature_template` SET `DamageMultiplier` = '4' WHERE `entry` = '3794';
-- Druid of the Claw 3795 
UPDATE `creature_template` SET `DamageMultiplier` = '4' WHERE `entry` = '3795';
-- Night Elf Archer 17943 
UPDATE `creature_template` SET `DamageMultiplier` = '16' WHERE `entry` = '17943';
-- Dryad 17944 
UPDATE `creature_template` SET `DamageMultiplier` = '4.44' WHERE `entry` = '17944';
-- Night Elf Huntress 17945 
UPDATE `creature_template` SET `DamageMultiplier` = '16' WHERE `entry` = '17945';
-- Night Elf Ancient Protector 18487 
UPDATE `creature_template` SET `DamageMultiplier` = '2.6' WHERE `entry` = '18487';
-- Night Elf Wisp 18502 
UPDATE `creature_template` SET `DamageMultiplier` = '15' WHERE `entry` = '18502';
-- Tyrande Whisperwind 17948 
UPDATE `creature_template` SET `DamageMultiplier` = '30' WHERE `entry` = '17948';

-- ============================================================================================================================================================================
-- Black Temple
-- ============================================================================================================================================================================

-- High Warlord Naj'entus 22887 
UPDATE `creature_template` SET `HealthMultiplier` = '500', `MinLevelHealth` = '3794000', `MaxLevelHealth` = '3794000', `DamageMultiplier` = '40' WHERE `entry` = '22887';
-- Supremus 22898 
UPDATE `creature_template` SET `HealthMultiplier` = '600', `MinLevelHealth` = '4552800', `MaxLevelHealth` = '4552800', `DamageMultiplier` = '70' WHERE `entry` = '22898';
-- Shade of Akama 22841 
UPDATE `creature_template` SET `HealthMultiplier` = '132', `MinLevelHealth` = '1001616', `MaxLevelHealth` = '1001616', `DamageMultiplier` = '94' WHERE `entry` = '22841';
-- Ashtongue Channeler 23421 
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '125675', `MaxLevelHealth` = '125675', `DamageMultiplier` = '15' WHERE `entry` = '23421';
-- Ashtongue Sorcerer 23215 
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '100540', `MaxLevelHealth` = '100540', `DamageMultiplier` = '27.4' WHERE `entry` = '23215';
-- Ashtongue Defender 23216 
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '88560', `MaxLevelHealth` = '88560', `DamageMultiplier` = '20' WHERE `entry` = '23216';
-- Ashtongue Elementalist 23523 
-- Ashtongue Spiritbinder 23524 
-- Ashtongue Rogue 23318 
-- Teron Gorefiend (22871) (577.5H/80D)
UPDATE `creature_template` SET `HealthMultiplier` = '825', `MinLevelHealth` = '5007750', `MaxLevelHealth` = '5007750', `DamageMultiplier` = '80' WHERE `entry` = '22871';
-- Shadowy Construct 23111+
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '59381', `MaxLevelHealth` = '59381' WHERE `entry` = '23111';
-- Gurtogg Bloodboil 22948 
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '5691000', `MaxLevelHealth` = '5691000', `DamageMultiplier` = '42' WHERE `entry` = '22948';
-- Essence of Suffering (23418) (210H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '300', `MinLevelHealth` = '2276400', `MaxLevelHealth` = '2276400', `DamageMultiplier` = '4' WHERE `entry` = '23418';
-- Essence of Desire (23419) (280H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '400', `MinLevelHealth` = '3035200', `MaxLevelHealth` = '3035200', `DamageMultiplier` = '32' WHERE `entry` = '23419';
-- Essence of Anger (23420) (280H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '400', `MinLevelHealth` = '3035200', `MaxLevelHealth` = '3035200', `DamageMultiplier` = '32' WHERE `entry` = '23420';
-- Enslaved Soul 23469 
-- Mother Shahraz 22947 
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '4552500', `MaxLevelHealth` = '4552500', `DamageMultiplier` = '54' WHERE `entry` = '22947';
-- The Illidari Council 23426 (700H - 4890200)
UPDATE `creature_template` SET `HealthMultiplier` = '1000', `MinLevelHealth` = '6986000', `MaxLevelHealth` = '6986000' WHERE `entry` = '23426'; -- 6449500 (Bestiary 2.3)
-- Gathios the Shatterer 22949 
UPDATE `creature_template` SET `HealthMultiplier` = '250', `MinLevelHealth` = '1517500', `MaxLevelHealth` = '1517500', `DamageMultiplier` = '75' WHERE `entry` = '22949';
-- High Nethermancer Zerevor 22950 
UPDATE `creature_template` SET `HealthMultiplier` = '250', `MinLevelHealth` = '1517500', `MaxLevelHealth` = '1517500', `DamageMultiplier` = '16' WHERE `entry` = '22950';
-- Lady Malande 22951 
UPDATE `creature_template` SET `HealthMultiplier` = '250', `MinLevelHealth` = '1517500', `MaxLevelHealth` = '1517500', `DamageMultiplier` = '25' WHERE `entry` = '22951';
-- Veras Darkshadow 22952
UPDATE `creature_template` SET `HealthMultiplier` = '250', `MinLevelHealth` = '1897000', `MaxLevelHealth` = '1897000', `DamageMultiplier` = '38' WHERE `entry` = '22952';
-- Illidan Stormrage 22917 
UPDATE `creature_template` SET `HealthMultiplier` = '800', `MinLevelHealth` = '6070400', `MaxLevelHealth` = '6070400', `DamageMultiplier` = '94' WHERE `entry` = '22917';
-- Flame of Azzinoth 22997 
UPDATE `creature_template` SET `HealthMultiplier` = '150', `MinLevelHealth` = '1138200', `MaxLevelHealth` = '1138200', `DamageMultiplier` = '38' WHERE `entry` = '22997';
-- Parasitic Shadowfiend 23498 
UPDATE `creature_template` SET `HealthMultiplier` = '0.5', `MinLevelHealth` = '3493', `MaxLevelHealth` = '3493', `DamageMultiplier` = '1.5' WHERE `entry` = '23498';
-- Shadow Demon 23375 
UPDATE `creature_template` SET `HealthMultiplier` = '3', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `DamageMultiplier` = '1' WHERE `entry` = '23375';
-- Akama 23089 (5D)
UPDATE `creature_template` SET `HealthMultiplier` = '165', `MinLevelHealth` = '1001550', `MaxLevelHealth` = '1001550', `DamageMultiplier` = '5' WHERE `entry` = '23089';
-- Illidari Elite 23226 
UPDATE `creature_template` SET `HealthMultiplier` = '3', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `DamageMultiplier` = '5' WHERE `entry` = '23226';
-- Maiev Shadowsong 23197 
UPDATE `creature_template` SET `HealthMultiplier` = '10.5', `MinLevelHealth` = '63735', `MaxLevelHealth` = '63735', `DamageMultiplier` = '6' WHERE `entry` = '23197';

-- ============================
-- Trash
-- ============================

-- Ashtongue Battlelord 22844 (45H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '332100', `MaxLevelHealth` = '332100', `DamageMultiplier` = '30' WHERE `entry` = '22844';
-- Ashtongue Mystic 22845 (30H/18D)
-- Ashtongue Stormcaller 22846 (30H/16D)
-- Ashtongue Primalist 22847 (21H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090', `DamageMultiplier` = '26' WHERE `entry` = '22847';
-- Storm Fury 22848 (10.5H/2.4D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '104790', `MaxLevelHealth` = '104790', `DamageMultiplier` = '2.4' WHERE `entry` = '22848';
-- Ashtongue Feral Spirit 22849 (15H/26D)
-- Illidari Defiler 22853 (30H/18D)
-- Illidari Nightlord 22855 (60H/35D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '442800', `MaxLevelHealth` = '442800', `DamageMultiplier` = '35' WHERE `entry` = '22855';
-- Illidari Boneslicer 22869 (30H/16D)
-- Coilskar General 22873 (40H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '28' WHERE `entry` = '22873';
-- Coilskar Harpooner 22874 (30H/20D)
-- Coilskar Sea-Caller 22875 (30H/16D)
-- Coilskar Soothsayer 22876 (30H/16D)
-- Coilskar Wrangler 22877 (30H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '30' WHERE `entry` = '22877';
-- Aqueous Lord 22878 (60H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '95', `MinLevelHealth` = '701100', `MaxLevelHealth` = '701100', `DamageMultiplier` = '26' WHERE `entry` = '22878';
-- Shadowmoon Reaver 22879 (30H/26D)
-- Shadowmoon Champion 22880 (45H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '332100', `MaxLevelHealth` = '332100', `DamageMultiplier` = '30' WHERE `entry` = '22880';
-- Aqueous Surger 22881 (15H/16.75D)
-- Shadowmoon Deathshaper 22882 (21H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090', `DamageMultiplier` = '18' WHERE `entry` = '22882';
-- Shadowmoon Fallen 23371 (15H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '107715', `MaxLevelHealth` = '107715', `DamageMultiplier` = '15' WHERE `entry` = '23371';
-- Aqueous Spawn 22883 (9H/9.3D)
-- Leviathan 22884 (75H/43.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '90', `MinLevelHealth` = '664200', `MaxLevelHealth` = '664200', `DamageMultiplier` = '43.5' WHERE `entry` = '22884';
-- Dragon Turtle 22885 (15H/25D)
-- Greater Shadowfiend 22929 (0.35H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.5', `MinLevelHealth` = '2952', `MaxLevelHealth` = '2952', `DamageMultiplier` = '3' WHERE `entry` = '22929';
-- Temple Concubine 22939 (3.15H/20D) - unitclass wrong?
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `DamageMultiplier` = '20' WHERE `entry` = '22939';
-- Shadowmoon Blood Mage 22945 (25H/18D)
-- Shadowmoon War Hound 22946 (7H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '71810', `MaxLevelHealth` = '71810', `DamageMultiplier` = '9' WHERE `entry` = '22946';
-- Wrathbone Flayer 22953 (50H/35D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '359050', `MaxLevelHealth` = '359050', `DamageMultiplier` = '35' WHERE `entry` = '22953';
-- Illidari Fearbringer 22954 (75H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '75', `MinLevelHealth` = '553500', `MaxLevelHealth` = '553500', `DamageMultiplier` = '30' WHERE `entry` = '22954';
-- Charming Courtesan 22955 (3.15H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `MinLevelHealth` = '25151', `MaxLevelHealth` = '25151', `DamageMultiplier` = '4' WHERE `entry` = '22955';
-- Sister of Pain 22956 (50H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150', `DamageMultiplier` = '30' WHERE `entry` = '22956';
-- Priestess of Dementia 22957 (90H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '90', `MinLevelHealth` = '531270', `MaxLevelHealth` = '531270', `DamageMultiplier` = '26' WHERE `entry` = '22957';
-- Spellbound Attendant 22959 (30H/26D)
-- Dragonmaw Wyrmcaller 22960 (35H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '28' WHERE `entry` = '22960';
-- Priestess of Delight 22962 (90H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '90', `MinLevelHealth` = '664200', `MaxLevelHealth` = '664200', `DamageMultiplier` = '40' WHERE `entry` = '22962';
-- Bonechewer Worker 22963 (4.5H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `MinLevelHealth` = '25151', `MaxLevelHealth` = '25151', `DamageMultiplier` = '4' WHERE `entry` = '22963';
-- Sister of Pleasure 22964 (50H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150', `DamageMultiplier` = '30' WHERE `entry` = '22964';
-- Enslaved Servant 22965 (17.5H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '179525', `MaxLevelHealth` = '179525', `DamageMultiplier` = '26' WHERE `entry` = '22965';
-- Shadowmoon Houndmaster 23018 (35H/30D)
-- Bonechewer Taskmaster 23028 (30H/24D)
-- Dragonmaw Sky Stalker 23030 (35H/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '16' WHERE `entry` = '23030';
-- Shadowmoon Soldier 23047 (5H/5D)
-- Shadowmoon Weapon Master 23049 (50H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '369000', `MaxLevelHealth` = '369000', `DamageMultiplier` = '40' WHERE `entry` = '23049';
-- Shadowmoon Riding Hound 23083 (20H/15D)
-- Shadowmoon Grunt 23147 (4.5H/4D)
-- Hand of Gorefiend 23172+ (24.5H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '258300', `MaxLevelHealth` = '258300', `DamageMultiplier` = '30' WHERE `entry` = '23172';
-- Bonechewer Behemoth 23196 (90H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '90', `MinLevelHealth` = '664200', `MaxLevelHealth` = '664200', `DamageMultiplier` = '30' WHERE `entry` = '23196';
-- Bonechewer Brawler 23222 (55H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '55', `MinLevelHealth` = '394955', `MaxLevelHealth` = '394955', `DamageMultiplier` = '40' WHERE `entry` = '23222';
-- Bonechewer Spectator 23223 (4.5H/4D)
-- Mutant War Hound 23232 (20H/14D)
-- Bonechewer Blade Fury 23235 (30H/24D)
-- Bonechewer Shield Deciple 23236 (30H/27.5D)
-- Bonechewer Blood Prophet 23237 (30H/18D)
-- Bonechewer Combatant 23239 (55H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '55', `MinLevelHealth` = '394955', `MaxLevelHealth` = '394955', `DamageMultiplier` = '40' WHERE `entry` = '23239';
-- Dragonmaw Wind Reaver 23330 (35H/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '14' WHERE `entry` = '23330';
-- Illidari Centurion 23337 (40H/26D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '295200', `MaxLevelHealth` = '295200', `DamageMultiplier` = '26' WHERE `entry` = '23337';
-- Illidari Heartseeker 23339 (30H/18D)
-- Whirling Blade 23369 (28H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '28', `MinLevelHealth` = '201068', `MaxLevelHealth` = '201068', `DamageMultiplier` = '10' WHERE `entry` = '23369';
-- Ashtongue Stalker 23374 (30H/26D)
-- Fallen Ally 23389 (14H/1D)
-- Promenade Sentinel 23394 (90H/32D)
UPDATE `creature_template` SET `HealthMultiplier` = '90', `MinLevelHealth` = '664200', `MaxLevelHealth` = '664200', `DamageMultiplier` = '32' WHERE `entry` = '23394';
-- Illidari Blood Lord 23397 (40H/29D)
-- Angered Soul Fragment 23398+ (0.7H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '5589', `MaxLevelHealth` = '5589', `DamageMultiplier` = '5' WHERE `entry` = '23398';
-- Suffering Soul Fragment 23399+ (8.4H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '70836', `MaxLevelHealth` = '70836', `DamageMultiplier` = '12' WHERE `entry` = '23399';
-- Illidari Archon 23400 (30H/20D)
-- Hungering Soul Fragment 23401 (4.2H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `MinLevelHealth` = '44280', `MaxLevelHealth` = '44280', `DamageMultiplier` = '9' WHERE `entry` = '23401';
-- Illidari Battle-Mage 23402 (30H/20D)
-- Illidari Assassin 23403 (25H/25D)
-- Image of Dementia 23436 (5.6H/0.013D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `MinLevelHealth` = '47224', `MaxLevelHealth` = '47224', `DamageMultiplier` = '0.013' WHERE `entry` = '23436';

-- ============================================================================================================================================================================
-- Zul'Aman
-- ============================================================================================================================================================================

-- Akil'zon 23574+ (154H/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '220', `MinLevelHealth` = '1335400', `MaxLevelHealth` = '1335400', `DamageMultiplier` = '14' WHERE `entry` = '23574';
-- Soaring Eagle 24858+ (0.35H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.5', `MinLevelHealth` = '3493', `MaxLevelHealth` = '3493', `DamageMultiplier` = '1' WHERE `entry` = '24858';
-- Nalorakk 23576+ (126.35H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '180', `MinLevelHealth` = '1365840', `MaxLevelHealth` = '1365840', `DamageMultiplier` = '20' WHERE `entry` = '23576';
-- Jan'alai 23578+ (94.5H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '135', `MinLevelHealth` = '1024380', `MaxLevelHealth` = '1024380', `DamageMultiplier` = '18' WHERE `entry` = '23578';
-- Amani'shi Hatcher 23818+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '1' WHERE `entry` = '23818';
-- Amani'shi Hatcher 24504+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '1' WHERE `entry` = '24504';
-- Amani Dragonhawk Hatchling 23598+ (0.7H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '5' WHERE `entry` = '23598';
-- Halazzi 23577+ (56.875H/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '80', `MinLevelHealth` = '607040', `MaxLevelHealth` = '607040', `DamageMultiplier` = '17' WHERE `entry` = '23577';
-- Spirit of the Lynx 24143+ (30.5H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '30.5', `MinLevelHealth` = '231434', `MaxLevelHealth` = '231434', `DamageMultiplier` = '20' WHERE `entry` = '24143';
-- Corrupted Lightning Totem 24224 (1H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '4399', `MaxLevelHealth` = '4399', `DamageMultiplier` = '1' WHERE `entry` = '24224';
-- Hex Lord Malacrass 24239+ (94.5H/18D)
UPDATE `creature_template` SET `HealthMultiplier` = '135', `MinLevelHealth` = '819450', `MaxLevelHealth` = '819450', `DamageMultiplier` = '18' WHERE `entry` = '24239';
-- Alyson Antille 24240+ (12.25H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '17.5', `MinLevelHealth` = '97808', `MaxLevelHealth` = '97808', `DamageMultiplier` = '5' WHERE `entry` = '24240';
-- Thurg 24241+ (9.8H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '97804', `MaxLevelHealth` = '97804', `DamageMultiplier` = '10' WHERE `entry` = '24241';
-- Slither 24242+ (9.8H/7,5D)
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '97804', `MaxLevelHealth` = '97804', `DamageMultiplier` = '7.5' WHERE `entry` = '24242';
-- Lord Raadan 24243+ (9.8H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '97804', `MaxLevelHealth` = '97804', `DamageMultiplier` = '8' WHERE `entry` = '24243';
-- Gazakroth 24244+ (12.25H/0,4D)
UPDATE `creature_template` SET `HealthMultiplier` = '17.5', `MinLevelHealth` = '97808', `MaxLevelHealth` = '97808', `DamageMultiplier` = '0.4' WHERE `entry` = '24244';
-- Fenstalker 24245+ (9.8H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '97804', `MaxLevelHealth` = '97804', `DamageMultiplier` = '8' WHERE `entry` = '24245';
-- Darkheart 24246+ (9.8H/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '97804', `MaxLevelHealth` = '97804', `DamageMultiplier` = '13' WHERE `entry` = '24246';
-- Koragg 24247+ (9.8H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '97804', `MaxLevelHealth` = '97804', `DamageMultiplier` = '10' WHERE `entry` = '24247';
-- Zul'jin 23863+ (157.5H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '225', `MinLevelHealth` = '1707300', `MaxLevelHealth` = '1707300', `DamageMultiplier` = '20' WHERE `entry` = '23863';

-- ============================
-- Trash
-- ============================

-- Amani'shi Axe Thrower 23542+ (8.4H/7D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '12' WHERE `entry` = '23542';
-- Amani'shi Warbringer 23580+ (10.5H/20.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '107715', `MaxLevelHealth` = '107715', `DamageMultiplier` = '24.5' WHERE `entry` = '23580';
-- Amani'shi Medicine Man 23581+ (8.4H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '68928', `MaxLevelHealth` = '68928', `DamageMultiplier` = '10' WHERE `entry` = '23581';
-- Amani'shi Tribesman 23582+ (8.4H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '15' WHERE `entry` = '23582';
-- Amani Bear 23584+ (8.4H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '83832', `MaxLevelHealth` = '83832', `DamageMultiplier` = '20' WHERE `entry` = '23584';
-- Amani'shi Scout 23586+ (0.7H/2D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '2' WHERE `entry` = '23586';
-- Amani'shi Reinforcement 23587+ (8.4H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '83832', `MaxLevelHealth` = '83832', `DamageMultiplier` = '20' WHERE `entry` = '23587';
-- Amani'shi Flame Caster 23596+ (8.4H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '68928', `MaxLevelHealth` = '68928', `DamageMultiplier` = '16' WHERE `entry` = '23596';
-- Amani'shi Guardian 23597+ (8.4H/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '21' WHERE `entry` = '23597';
-- Amani'shi Trainer 23774 (10.5H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '104790', `MaxLevelHealth` = '104790', `DamageMultiplier` = '18' WHERE `entry` = '23774';
-- Amani Dragonhawk 23834+ (2.1H/11D)
UPDATE `creature_template` SET `HealthMultiplier` = '3', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `DamageMultiplier` = '11' WHERE `entry` = '23834';
-- Amani'shi Savage 23889+ (0.28H/1.2D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.4', `MinLevelHealth` = '2794', `MaxLevelHealth` = '2794', `DamageMultiplier` = '1.2' WHERE `entry` = '23889';
-- Amani Lynx 24043+ (5.6H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `MinLevelHealth` = '55888', `MaxLevelHealth` = '55888', `DamageMultiplier` = '10' WHERE `entry` = '24043';
-- Amani Crocolisk 24047+ (2.8H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '4', `MinLevelHealth` = '27944', `MaxLevelHealth` = '27944', `DamageMultiplier` = '10' WHERE `entry` = '24047';
-- Amani'shi Beast Tamer 24059+ (8.4H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '18' WHERE `entry` = '24059';
-- Amani Lynx Cub 24064+ (2.1H/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '4', `MinLevelHealth` = '27944', `MaxLevelHealth` = '27944', `DamageMultiplier` = '4' WHERE `entry` = '24064';
-- Amani'shi Handler 24065+ (8.4H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '18' WHERE `entry` = '24065';
-- Tamed Amani Crocolisk 24138+ (2.8H/100D)
UPDATE `creature_template` SET `HealthMultiplier` = '4', `MinLevelHealth` = '27944', `MaxLevelHealth` = '27944', `DamageMultiplier` = '10' WHERE `entry` = '24138';
-- Amani Eagle 24159+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '1' WHERE `entry` = '24159';
-- Amani'shi Wind Walker 24179+ (7H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '57440', `MaxLevelHealth` = '57440', `DamageMultiplier` = '18' WHERE `entry` = '24179';
-- Amani'shi Protector 24180+ (7H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '71810', `MaxLevelHealth` = '71810', `DamageMultiplier` = '20' WHERE `entry` = '24180';
-- Amani Bear Mount 24217+ (4.2H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `DamageMultiplier` = '15' WHERE `entry` = '24217';
-- Amani'shi Warrior 24225+ (2.45H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.5', `MinLevelHealth` = '24451', `MaxLevelHealth` = '24451', `DamageMultiplier` = '10' WHERE `entry` = '24225';
-- Amani Snake 24338 
-- Amani'shi Berserker 24374+ (14H/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '139720', `MaxLevelHealth` = '139720', `DamageMultiplier` = '25' WHERE `entry` = '24374';
-- Amani Elder Lynx 24530+ (5.6H/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `MinLevelHealth` = '55888', `MaxLevelHealth` = '55888', `DamageMultiplier` = '20' WHERE `entry` = '24530';
-- Amani'shi Tempest 24549+ (14H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '143620', `MaxLevelHealth` = '143620', `DamageMultiplier` = '27' WHERE `entry` = '24549';

-- ============================================================================================================================================================================
-- Sunwell Plateau
-- ============================================================================================================================================================================

-- Kalecgos 24850+ (332.5H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '475', `MinLevelHealth` = '2883250', `MaxLevelHealth` = '2883250', `DamageMultiplier` = '40' WHERE `entry` = '24850';
-- Sathrovarr the Corruptor 24892+ (332.5H/50D)
UPDATE `creature_template` SET `HealthMultiplier` = '475', `MinLevelHealth` = '2883250', `MaxLevelHealth` = '2883250', `DamageMultiplier` = '50' WHERE `entry` = '24892';
-- Kalecgos 24891+ (136.5H/1.3D)
UPDATE `creature_template` SET `HealthMultiplier` = '195', `MinLevelHealth` = '1183650', `MaxLevelHealth` = '1183650', `DamageMultiplier` = '5' WHERE `entry` = '24891';
-- Brutallus 24882+ (968.8H/111D)
UPDATE `creature_template` SET `HealthMultiplier` = '1384', `MinLevelHealth` = '10501792', `MaxLevelHealth` = '10501792', `DamageMultiplier` = '111' WHERE `entry` = '24882';
-- Felmyst 25038+ (807.8H/41D)
UPDATE `creature_template` SET `HealthMultiplier` = '1154', `MinLevelHealth` = '7004780', `MaxLevelHealth` = '7004780', `DamageMultiplier` = '41' WHERE `entry` = '25038';
-- Unyielding Dead 25268+ (3.5H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `DamageMultiplier` = '8' WHERE `entry` = '25268';
-- Lady Sacrolash 25165+ (332.5H/21D)
UPDATE `creature_template` SET `HealthMultiplier` = '475', `MinLevelHealth` = '2883250', `MaxLevelHealth` = '2883250', `DamageMultiplier` = '21' WHERE `entry` = '25165';
-- Grand Warlock Alythess 25166+ (332.5H/21D)
UPDATE `creature_template` SET `HealthMultiplier` = '475', `MinLevelHealth` = '2883250', `MaxLevelHealth` = '2883250', `DamageMultiplier` = '21' WHERE `entry` = '25166';
-- M'uru 25741+ (252H/9D) - health decreased in 2.4.3 to 360 (2731680) from 400 (3035200)
UPDATE `creature_template` SET `HealthMultiplier` = '360.0', `MinLevelHealth` = '2731680', `MaxLevelHealth` = '2731680', `DamageMultiplier` = '9' WHERE `entry` = '25741';
-- Entropius 25840+ (189H/11D) - health decreased in 2.4.3 to 270 (2048760) from 300 (2276400)
UPDATE `creature_template` SET `HealthMultiplier` = '270.0', `MinLevelHealth` = '2048760', `MaxLevelHealth` = '2048760', `DamageMultiplier` = '55' WHERE `entry` = '25840'; -- D should be ~80
-- Shadowsword Berserker 25798+ (11.97H/10D) - health decreased in 2.4.3 to 17.25
UPDATE `creature_template` SET `HealthMultiplier` = '19', `MinLevelHealth` = '136439', `MaxLevelHealth` = '136439', `DamageMultiplier` = '20' WHERE `entry` = '25798'; -- D should be 40
-- Shadowsword Fury Mage 25799+ (12.6H/8D) - health decreased in 2.4.3 to 18
UPDATE `creature_template` SET `HealthMultiplier` = '20.0', `MinLevelHealth` = '114880', `MaxLevelHealth` = '114880', `DamageMultiplier` = '16' WHERE `entry` = '25799'; -- D should be 32
-- Void Sentinel 25772+ (17.5H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '174650', `MaxLevelHealth` = '174650', `DamageMultiplier` = '40' WHERE `entry` = '25772'; -- D should be ~80
-- Void Sentinal Summoner+ 25782 (0.945H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.35', `MinLevelHealth` = '9431', `MaxLevelHealth` = '9431', `DamageMultiplier` = '1' WHERE `entry` = '25782';
-- Void Spawn 25824+ (2.4349H/2.3D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.4784', `MinLevelHealth` = '24300', `MaxLevelHealth` = '24300', `DamageMultiplier` = '2.3' WHERE `entry` = '25824';
-- Dark Fiend 25744+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '4050', `MaxLevelHealth` = '4050', `DamageMultiplier` = '1' WHERE `entry` = '25744';
-- Darkness 25879+ (0.7H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986', `DamageMultiplier` = '1' WHERE `entry` = '25879';
-- Kil'jaeden <The Deceiver> 25315+ (1540H/18.75D)
UPDATE `creature_template` SET `HealthMultiplier` = '2200', `MinLevelHealth` = '13354000', `MaxLevelHealth` = '13354000', `DamageMultiplier` = '18.75' WHERE `entry` = '25315';
-- Hand of the Deceiver 25588+ (24.5H/7D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '206605', `MaxLevelHealth` = '206605', `DamageMultiplier` = '28' WHERE `entry` = '25588';
-- Felfire Portal 25603+ (0.007H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.01', `MinLevelHealth` = '43', `MaxLevelHealth` = '43', `DamageMultiplier` = '1' WHERE `entry` = '25603';
-- Volatile Felfire Fiend 25598+ (0.182H/9.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.26', `MinLevelHealth` = '1493', `MaxLevelHealth` = '1493', `DamageMultiplier` = '9.5' WHERE `entry` = '25598';
-- Sinister Reflection 25708+ (14H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '111780', `MaxLevelHealth` = '111780', `DamageMultiplier` = '30' WHERE `entry` = '25708';
-- Shield Orb 25502+ (2.002H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.86', `MinLevelHealth` = '19980', `MaxLevelHealth` = '19980', `DamageMultiplier` = '1' WHERE `entry` = '25502';
-- Kalecgos 25319+ (297.5H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '425', `MinLevelHealth` = '2579750', `MaxLevelHealth` = '2579750', `DamageMultiplier` = '1' WHERE `entry` = '25319';

-- ============================
-- Trash
-- ============================

-- Sunblade Cabalist 25363+ (24.5H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '195615', `MaxLevelHealth` = '195615', `DamageMultiplier` = '20' WHERE `entry` = '25363';
-- Sunblade Arch Mage 25367+ (24.5H/1.75D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '195615', `MaxLevelHealth` = '195615', `DamageMultiplier` = '20' WHERE `entry` = '25367';
-- Sunblade Slayer 25368+ (24.5H/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '244510', `MaxLevelHealth` = '244510', `DamageMultiplier` = '42' WHERE `entry` = '25368';
-- Sunblade Vindicator 25369+ (24.5H/35D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '244510', `MaxLevelHealth` = '244510', `DamageMultiplier` = '70' WHERE `entry` = '25369';
-- Sunblade Dusk Priest 25370+ (24.5H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '195615', `MaxLevelHealth` = '195615', `DamageMultiplier` = '20' WHERE `entry` = '25370';
-- Sunblade Dawn Priest 25371+ (24.5H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '195615', `MaxLevelHealth` = '195615', `DamageMultiplier` = '20' WHERE `entry` = '25371';
-- Sunblade Scout 25372+ (2.1H/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '3', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `DamageMultiplier` = '27' WHERE `entry` = '25372';
-- Shadowsword Soulbinder 25373+ (24.5H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '20' WHERE `entry` = '25373';
-- Shadowsword Manafiend 25483+ (24.5H/10.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '25' WHERE `entry` = '25483';
-- Shadowsword Assassin 25484+ (24.5H/25D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '251335', `MaxLevelHealth` = '251335', `DamageMultiplier` = '50' WHERE `entry` = '25484';
-- Shadowsword Deathbringer 25485+ (24.5H/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '251335', `MaxLevelHealth` = '251335', `DamageMultiplier` = '34' WHERE `entry` = '25485';
-- Shadowsword Vanquisher 25486+ (24.5H/42D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '251335', `MaxLevelHealth` = '251335', `DamageMultiplier` = '84' WHERE `entry` = '25486';
-- Shadowsword Lifeshaper 25506+ (24.5H/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '20' WHERE `entry` = '25506';
-- Sunblade Protector 25507+ (52.5H/30D)
UPDATE `creature_template` SET `HealthMultiplier` = '75', `MinLevelHealth` = '553500', `MaxLevelHealth` = '553500', `DamageMultiplier` = '90' WHERE `entry` = '25507';
-- Shadowsword Guardian 25508+ (70H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `MinLevelHealth` = '738000', `MaxLevelHealth` = '738000', `DamageMultiplier` = '105' WHERE `entry` = '25508';
-- Priestess of Torment 25509+ (24.5H/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '251335', `MaxLevelHealth` = '251335', `DamageMultiplier` = '24' WHERE `entry` = '25509';
-- Painbringer 25591+ (24.5H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '251335', `MaxLevelHealth` = '251335', `DamageMultiplier` = '52' WHERE `entry` = '25591';
-- Doomfire Destroyer 25592+ (28H/40D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '287240', `MaxLevelHealth` = '287240', `DamageMultiplier` = '80' WHERE `entry` = '25592';
-- Apocalypse Guard 25593+ (48.125H/18.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '68.75', `MinLevelHealth` = '493694', `MaxLevelHealth` = '493694', `DamageMultiplier` = '41' WHERE `entry` = '25593';
-- Chaos Gazer 25595+ (53.2H/11.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '76', `MinLevelHealth` = '545756', `MaxLevelHealth` = '545756', `DamageMultiplier` = '35' WHERE `entry` = '25595';
-- Oblivion Mage 25597+ (24.5H/11.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '201040', `MaxLevelHealth` = '201040', `DamageMultiplier` = '27' WHERE `entry` = '25597';
-- Cataclysm Hound 25599+ (60.2H/22.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '86', `MinLevelHealth` = '617566', `MaxLevelHealth` = '617566', `DamageMultiplier` = '45' WHERE `entry` = '25599';
-- Shadowsword Commander 25837+ (24.5H/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '251335', `MaxLevelHealth` = '251335', `DamageMultiplier` = '34' WHERE `entry` = '25837';
-- Volatile Fiend 25851+ (1.4H/6.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `MinLevelHealth` = '11488', `MaxLevelHealth` = '11488', `DamageMultiplier` = '6.5' WHERE `entry` = '25851';
-- Blazing Infernal 25860+ (9.1H/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '13', `MinLevelHealth` = '90818', `MaxLevelHealth` = '90818', `DamageMultiplier` = '12' WHERE `entry` = '25860';
-- Felguard Slayer 25864+ (9.1H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '13', `MinLevelHealth` = '90818', `MaxLevelHealth` = '90818', `DamageMultiplier` = '18',
`MinLevel` = '70', `MaxLevel` = '70', `Expansion` = '1' WHERE `entry` = '25864';
-- Sunblade Dragonhawk 25867+ (14H/12.25D)
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '139720', `MaxLevelHealth` = '139720', `DamageMultiplier` = '24.5' WHERE `entry` = '25867';
-- Doomfire Shard 25948+ (5.6H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '8', `MinLevelHealth` = '57448', `MaxLevelHealth` = '57448', `DamageMultiplier` = '20' WHERE `entry` = '25948';
-- Fire Fiend 26101+ (1.4H/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '2', `MinLevelHealth` = '11178', `MaxLevelHealth` = '11178', `DamageMultiplier` = '3' WHERE `entry` = '26101';

-- ============================================================================================================================================================================
-- ============================================================================================================================================================================

-- 																================================
-- 																=====     TBC DUNGEONS     =====
-- 																================================

-- ============================================================================================================================================================================
-- Auchindoun: Mana-Tombs
-- ============================================================================================================================================================================

-- Ethereal Scavenger 18309,20258 (5/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3535', `MaxMeleeDmg` = '4998' WHERE `entry` = '20258';
-- Ethereal Crypt Raider 18311,20255 (4.5/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3535', `MaxMeleeDmg` = '4998' WHERE `entry` = '20255';
-- Ethereal Spellbinder 18312,20260 (7.73/4.766D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `DamageMultiplier` = '6.0', `MinLevelHealth` = '20664', `MaxLevelHealth` = '20664', `MinMeleeDmg` = '959', `MaxMeleeDmg` = '1393' WHERE `entry` = '20260';
-- Ethereal Sorcerer 18313,20259 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '6.0', `MinLevelHealth` = '22621', `MaxLevelHealth` = '22621', `MinMeleeDmg` = '1226', `MaxMeleeDmg` = '1781' WHERE `entry` = '20259';
-- Nexus Stalker 18314,20264 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '19.0', `MinLevelHealth` = '35905', `MaxLevelHealth` = '36900', `MinMeleeDmg` = '3414', `MaxMeleeDmg` = '4909' WHERE `entry` = '20264';
-- Ethereal Theurgist 18315,20261 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '5.0', `MinLevelHealth` = '23247', `MaxLevelHealth` = '23247', `MinMeleeDmg` = '852', `MaxMeleeDmg` = '1238' WHERE `entry` = '20261';
-- Ethereal Priest 18317,20257 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '6.0', `MinLevelHealth` = '22621', `MaxLevelHealth` = '22621', `MinMeleeDmg` = '1104', `MaxMeleeDmg` = '1603' WHERE `entry` = '20257';
-- Ethereal Darkcaster 18331,20256 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '22621', `MaxLevelHealth` = '22621', `MinMeleeDmg` = '2453', `MaxMeleeDmg` = '3561' WHERE `entry` = '20256';
-- Pandemonius (18341,20267) (12/16.2H)(5.8/20D) - 73392/119556 - SPELL_SCHOOL_SHADOW
-- After further investigation it seems the tuned value we had when server started could be closer to his actual damage value, but seemed totally overtuned
-- https://gitlab.com/lights-vengeance/tbc-db-vengeance/-/commit/7aabeaa96c69c84f96ce3f588d605d704c34c23b
-- `DamageMultiplier` = '26.0', `MinMeleeDmg` = '4072', `MaxMeleeDmg` = '5757' seems closer to the real value he should have
-- https://tbc.wowhead.com/npc=18341/pandemonius#comments:id=5222853 - How can 4500 normal hit on the low (5.5k on high) end be normal?
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `DamageMultiplier` = '20.0', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980', `MinMeleeDmg` = '3550', `MaxMeleeDmg` = '4250' WHERE `entry` = '20267';
-- Tavarok (18343,20268) (15/20.25H)(5.8/24D) - 91740/149445
UPDATE `creature_template` SET `HealthMultiplier` = '28.0', `DamageMultiplier` = '28.0', `MinLevelHealth` = '206640', `MaxLevelHealth` = '206640', `MinMeleeDmg` = '4385', `MaxMeleeDmg` = '6201' WHERE `entry` = '20268';
-- Nexus-Prince Shaffar (18344,20266) (18/24.3H)(6/4.7D) - 88056/143443
UPDATE `creature_template` SET `HealthMultiplier` = '31.0', `DamageMultiplier` = '21.0', `MinLevelHealth` = '182993', `MaxLevelHealth` = '182993', `MinMeleeDmg` = '5082', `MaxMeleeDmg` = '7179' WHERE `entry` = '20266';
-- Ethereal Wraith 18394,20262+ (4/4D)
-- Arcane Fiend 18429,20252+ (2/5D)
-- Ethereal Apprentice 18430,20253+ (3/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `DamageMultiplier` = '5.0', `MinLevelHealth` = '8616', `MaxLevelHealth` = '8616', `MinMeleeDmg` = '1191', `MaxMeleeDmg` = '1683' WHERE `entry` = '20253';
-- Mana Leech 19306,20263 
UPDATE `creature_template` SET `HealthMultiplier` = '2.25', `DamageMultiplier` = '4.0', `MinLevelHealth` = '12575', `MaxLevelHealth` = '12575', `MinMeleeDmg` = '937', `MaxMeleeDmg` = '1324' WHERE `entry` = '20263';
-- Nexus Terror 19307,20265 (5/15D)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `DamageMultiplier` = '20.0', `MinLevelHealth` = '76846', `MaxLevelHealth` = '76846', `MinMeleeDmg` = '5050', `MaxMeleeDmg` = '7140' WHERE `entry` = '20265';
-- Shadow Lord Xiraxis 19666
UPDATE `creature_template` SET `HealthMultiplier` = '19.0', `DamageMultiplier` = '8.0', `MinLevelHealth` = '92948', `MaxLevelHealth` = '92948', `MinMeleeDmg` = '1560', `MaxMeleeDmg` = '2187' WHERE `entry` = '19666';
-- Cryo-Engineer Sha'heen 19671
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '2.5', `MinLevelHealth` = '29352', `MaxLevelHealth` = '29352', `MinMeleeDmg` = '487', `MaxMeleeDmg` = '684' WHERE `entry` = '19671';
-- Consortium Laborer 19672
-- Consortium Engineer 19673
-- Ambassador Pax'ivi 22928
-- Yor Void Hound of Shaffar (22930) (35H/15D) - 244343
UPDATE `creature_template` SET `HealthMultiplier` = '41.0', `DamageMultiplier` = '21.0', `MinLevelHealth` = '286426', `MaxLevelHealth` = '286426', `MinMeleeDmg` = '5302', `MaxMeleeDmg` = '7497' WHERE `entry` = '22930';

-- ============================================================================================================================================================================
-- Auchindoun: Auchenai Crypts
-- ============================================================================================================================================================================

-- Shirrak the Dead Watcher (18371,20318) (15.75/21.2625H)(6/10D) - 77409/122132
UPDATE `creature_template` SET `DamageMultiplier` = '3' WHERE `entry` = 18371;
UPDATE `creature_template` SET `HealthMultiplier` = '21.2625', `DamageMultiplier` = '10.0', `MinLevelHealth` = '122132', `MaxLevelHealth` = '122132', `MinMeleeDmg` = '3573', `MaxMeleeDmg` = '5049' WHERE `entry` = '20318';
-- Exarch Maladaar (18373,20306) (16.5/22.275H)(14/12D) - 97400/131489
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `DamageMultiplier` = '19.0', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090', `MinMeleeDmg` = '4598', `MaxMeleeDmg` = '6495' WHERE `entry` = '20306';
-- Stolen Soul 18441,20305 
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '6.0', `MinLevelHealth` = '14362', `MaxLevelHealth` = '14760', `MinMeleeDmg` = '1078', `MaxMeleeDmg` = '1550' WHERE `entry` = '20305';
-- Avatar of the Martyred (18478,20303) (4.75/6.4125H)(4.994/12.5D) - 30049/47324
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `DamageMultiplier` = '16.5', `MinLevelHealth` = '62730', `MaxLevelHealth` = '62730', `MinMeleeDmg` = '4306', `MaxMeleeDmg` = '6090' WHERE `entry` = '20303';
-- Auchenai Soulpriest 18493,20301 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4713' WHERE `entry` = '20301';
-- Auchenai Vindicator 18495,20302 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4713' WHERE `entry` = '20302';
-- Auchenai Monk 18497,20299 
UPDATE `creature_template` SET `HealthMultiplier` = '5.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '38423', `MaxLevelHealth` = '39495', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4357' WHERE `entry` = '20299';
-- Unliving Soldier 18498,20321 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '10.0', `MinLevelHealth` = '17465', `MaxLevelHealth` = '17952', `MinMeleeDmg` = '2525', `MaxMeleeDmg` = '3631' WHERE `entry` = '20321';
-- Unliving Sorcerer 18499,20322 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '6.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '14360', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '2020' WHERE `entry` = '20322';
-- Unliving Cleric 18500,20320 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '6.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '14360', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '2020' WHERE `entry` = '20320';
-- Unliving Stalker 18501,20323 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '11.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '14360', `MinMeleeDmg` = '2578', `MaxMeleeDmg` = '3703' WHERE `entry` = '20323';
-- Phantasmal Possessor 18503,20309 (0.3/0.8H)(2.5/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.8', `DamageMultiplier` = '5.0', `MinLevelHealth` = '3912', `MaxLevelHealth` = '4022', `MinMeleeDmg` = '960', `MaxMeleeDmg` = '1454' WHERE `entry` = '20309';
-- Raging Soul 18506,20316 
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `DamageMultiplier` = '5.0', `MinLevelHealth` = '8383', `MaxLevelHealth` = '8616', `MinMeleeDmg` = '1172', `MaxMeleeDmg` = '1683' WHERE `entry` = '20316';
-- Raging Skeleton 18521,20315 (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '20315';
-- Angered Skeleton 18524,20298 (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '20298';
-- Phasing Soldier 18556,20311 (1/1.15D)(2/7D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.15', `DamageMultiplier` = '7.0', `MinLevelHealth` = '8034', `MaxLevelHealth` = '8258', `MinMeleeDmg` = '2525', `MaxMeleeDmg` = '3631' WHERE `entry` = '20311';
-- Phasing Cleric 18557,20310 (1/1.15H)(2/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.15', `DamageMultiplier` = '4.0', `MinLevelHealth` = '6427', `MaxLevelHealth` = '6606', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '2020' WHERE `entry` = '20310';
-- Phasing Sorcerer 18558,20312 (1/1.15H)(2/5D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.15', `DamageMultiplier` = '5.0', `MinLevelHealth` = '6427', `MaxLevelHealth` = '6606', `MinMeleeDmg` = '1640', `MaxMeleeDmg` = '2356' WHERE `entry` = '20312';
-- Phasing Stalker 18559,20313 (1/1.15H)(2.57/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.15', `DamageMultiplier` = '9.0', `MinLevelHealth` = '6427', `MaxLevelHealth` = '6606', `MinMeleeDmg` = '2132', `MaxMeleeDmg` = '3063' WHERE `entry` = '20313';
-- Reanimated Bones 18700,20317 (0.54/0.729H)(1/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '0.729', `DamageMultiplier` = '4.0', `MinLevelHealth` = '5235', `MaxLevelHealth` = '5380', `MinMeleeDmg` = '1284', `MaxMeleeDmg` = '1845' WHERE `entry` = '20317';
-- Auchenai Necromancer 18702,20300 
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '45952', `MaxLevelHealth` = '47224', `MinMeleeDmg` = '3573', `MaxMeleeDmg` = '5128' WHERE `entry` = '20300';
-- Flying Raging Soul 18726,20307 
UPDATE `creature_template` SET `HealthMultiplier` = '1.75', `DamageMultiplier` = '3.0', `MinLevelHealth` = '8279', `MaxLevelHealth` = '8279', `MinMeleeDmg` = '5', `MaxMeleeDmg` = '5' WHERE `entry` = '20307';
-- D'ore 19412

-- ============================================================================================================================================================================
-- Auchindoun: Sethekk Halls
-- ============================================================================================================================================================================

-- Sethekk Initiate 18318,20693 
UPDATE `creature_template` SET `HealthMultiplier` = '4.75', `DamageMultiplier` = '15.0', `MinLevelHealth` = '33183', `MaxLevelHealth` = '33183', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '20693';
-- Time-Lost Scryer 18319,20697 
UPDATE `creature_template` SET `HealthMultiplier` = '4.75', `DamageMultiplier` = '13.0', `MinLevelHealth` = '23227', `MaxLevelHealth` = '23878', `MinMeleeDmg` = '2038', `MaxMeleeDmg` = '3087' WHERE `entry` = '20697';
-- Time-Lost Shadowmage 18320,20698 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '9.0', `MinLevelHealth` = '23247', `MaxLevelHealth` = '23247', `MinMeleeDmg` = '1725', `MaxMeleeDmg` = '2507' WHERE `entry` = '20698';
-- Sethekk Talon Lord 18321,20701 
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `DamageMultiplier` = '17.0', `MinLevelHealth` = '40208', `MaxLevelHealth` = '41321', `MinMeleeDmg` = '3645', `MaxMeleeDmg` = '5230' WHERE `entry` = '20701';
-- Sethekk Ravenguard 18322,20696 (2.9/6H)(4.66/14D) 6541.7/7181 
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '7.0', `MinLevelHealth` = '18971', `MaxLevelHealth` = '18971' WHERE `entry` = '18322';
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '21.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086' WHERE `entry` = '20696';
-- Sethekk Guard 18323,20692 (10.58/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `DamageMultiplier` = '24.0', `MinLevelHealth` = '48902', `MaxLevelHealth` = '50267', `MinMeleeDmg` = '6060', `MaxMeleeDmg` = '8714' WHERE `entry` = '20692';
-- Sethekk Prophet 18325,20695 
UPDATE `creature_template` SET `HealthMultiplier` = '4.75', `DamageMultiplier` = '16.0', `MinLevelHealth` = '23878', `MaxLevelHealth` = '24538', `MinMeleeDmg` = '2943', `MaxMeleeDmg` = '4457' WHERE `entry` = '20695';
-- Sethekk Shaman 18326,20699 
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '35418', `MaxLevelHealth` = '35418', `MinMeleeDmg` = '3098', `MaxMeleeDmg` = '4376' WHERE `entry` = '20699';
-- Time-Lost Controller 18327,20691 
UPDATE `creature_template` SET `HealthMultiplier` = '4.75', `DamageMultiplier` = '9.0', `MinLevelHealth` = '23227', `MaxLevelHealth` = '23227', `MinMeleeDmg` = '1587', `MaxMeleeDmg` = '2303' WHERE `entry` = '20691';
-- Sethekk Oracle 18328,20694 
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '19560', `MaxLevelHealth` = '20108', `MinMeleeDmg` = '2822', `MaxMeleeDmg` = '4274' WHERE `entry` = '20694';
-- Darkweaver Syth (18472,20690) (18/22H)(6.3/11D) - 85194/113652
UPDATE `creature_template` SET `HealthMultiplier` = '31.0', `DamageMultiplier` = '21.0', `MinLevelHealth` = '160146', `MaxLevelHealth` = '160146', `MinMeleeDmg` = '4473', `MaxMeleeDmg` = '6500' WHERE `entry` = '20690';
-- Talon King Ikiss (18473,20706) (17/21.9375H)(9.2/15D) - 80461/113329
UPDATE `creature_template` SET `HealthMultiplier` = '32.0', `DamageMultiplier` = '24.0', `MinLevelHealth` = '165312', `MaxLevelHealth` = '165312', `MinMeleeDmg` = '5112', `MaxMeleeDmg` = '7428' WHERE `entry` = '20706';
-- Dark Vortex 18701,20689 
-- Sethekk Spirit 18703
-- Lakka 18956
-- Syth Fire Elemental 19203,20703+ (2.32/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `DamageMultiplier` = '4.0', `MinLevelHealth` = '5903', `MaxLevelHealth` = '5903', `MinMeleeDmg` = '1016', `MaxMeleeDmg` = '1436' WHERE `entry` = '20703';
-- Syth Frost Elemental 19204,20704+ (2.32/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `DamageMultiplier` = '4.0', `MinLevelHealth` = '5903', `MaxLevelHealth` = '5903', `MinMeleeDmg` = '1016', `MaxMeleeDmg` = '1436' WHERE `entry` = '20704';
-- Syth Arcane Elemental 19205,20702+ (2.32/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `DamageMultiplier` = '4.0', `MinLevelHealth` = '5903', `MaxLevelHealth` = '5903', `MinMeleeDmg` = '1016', `MaxMeleeDmg` = '1436' WHERE `entry` = '20702';
-- Syth Shadow Elemental 19206,20705+ (2.32/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `DamageMultiplier` = '4.0', `MinLevelHealth` = '5903', `MaxLevelHealth` = '5903', `MinMeleeDmg` = '1016', `MaxMeleeDmg` = '1436' WHERE `entry` = '20705';
-- Cobalt Serpent 19428,20688 (6/7.425H)(6/15D) - 31398/41498
UPDATE `creature_template` SET `HealthMultiplier` = '14', `DamageMultiplier` = '20.0', `MinLevelHealth` = '80416', `MaxLevelHealth` = '80416', `MinMeleeDmg` = '4452', `MaxMeleeDmg` = '6396' WHERE `entry` = '20688';
-- Avian Darkhawk 19429,20686 
UPDATE `creature_template` SET `HealthMultiplier` = '6.5', `DamageMultiplier` = '17.5', `MinLevelHealth` = '45409', `MaxLevelHealth` = '45409', `MinMeleeDmg` = '4418', `MaxMeleeDmg` = '6248' WHERE `entry` = '20686';
-- Avian Ripper 21891,21989 
UPDATE `creature_template` SET `HealthMultiplier` = '1.75', `DamageMultiplier` = '7.0', `MinLevelHealth` = '11831', `MaxLevelHealth` = '12225', `MinMeleeDmg` = '1691', `MaxMeleeDmg` = '2499' WHERE `entry` = '21989';
-- Avian Warhawk 21904,21990 
UPDATE `creature_template` SET `HealthMultiplier` = '6.5', `DamageMultiplier` = '17.5', `MinLevelHealth` = '45409', `MaxLevelHealth` = '46676', `MinMeleeDmg` = '4418', `MaxMeleeDmg` = '6354' WHERE `entry` = '21990';
-- Avian Flyer 21931,21988 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '7.0', `MinLevelHealth` = '15815', `MaxLevelHealth` = '15815', `MinMeleeDmg` = '1543', `MaxMeleeDmg` = '2170' WHERE `entry` = '21988';
-- Anzu (23035) (40H/20D) - 236120
UPDATE `creature_template` SET `HealthMultiplier` = '50.0', `DamageMultiplier` = '26.0', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150', `MinMeleeDmg` = '6292', `MaxMeleeDmg` = '8888' WHERE `entry` = '23035';
-- Brood of Anzu 23132+ (1.5H/2D)
UPDATE `creature_template` SET `DamageMultiplier` = '3' WHERE `entry` = 23132;
-- Hawk Spirit 23134
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '7.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '1767', `MaxMeleeDmg` = '2499' WHERE `entry` = '23134';
-- Falcon Spirit 23135
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '7.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '1767', `MaxMeleeDmg` = '2499' WHERE `entry` = '23135';
-- Eagle Spirit 23136
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '7.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '1767', `MaxMeleeDmg` = '2499' WHERE `entry` = '23136';

-- ============================================================================================================================================================================
-- Auchindoun: Shadow Labyrinth
-- ============================================================================================================================================================================

-- Cabal Cultist 18631,20640 (2.6/3.51H)(4.66/12D) - 17579/24521
UPDATE `creature_template` SET `HealthMultiplier` = '2.6', `DamageMultiplier` = '5.5', `MinLevelHealth` = '17579', `MaxLevelHealth` = '18164', `MinMeleeDmg` = '1329', `MaxMeleeDmg` = '1964' WHERE `entry` = '18631';
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '4292', `MaxMeleeDmg` = '6172' WHERE `entry` = '20640';
-- Cabal Executioner 18632,20642 (3/5H)(4.66/12.5D) - 21543/35905
UPDATE `creature_template` SET `DamageMultiplier` = '6' WHERE `entry` = 18632;
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '15.5', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '5699', `MaxMeleeDmg` = '8060' WHERE `entry` = '20642';
-- Cabal Acolyte 18633,20638 (3/4.5H)(4.66/11.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5463' WHERE `entry` = '20638';
-- Cabal Summoner 18634,20648 (3/4.05H)(4.66/11.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5554' WHERE `entry` = '20648';
-- Cabal Deathsworn 18635,20641 (3/4.05H)(4.66/12D) - 20283/28293
UPDATE `creature_template` SET `DamageMultiplier` = '6' WHERE `entry` = 18635;
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '4292', `MaxMeleeDmg` = '6172' WHERE `entry` = '20641';
-- Cabal Assassin 18636,20639 (3/4.05H)(4.884/12.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '5.5', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `MinMeleeDmg` = '1250', `MaxMeleeDmg` = '1767' WHERE `entry` = '18636';
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '3749', `MaxMeleeDmg` = '5302' WHERE `entry` = '20639';
-- Cabal Shadow Priest 18637,20646 (3/4.05H)(4.66/11D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5554' WHERE `entry` = '20646';
-- Cabal Zealot 18638,20650 (3/4H)(4.25/10.5D) - 16767/22356
UPDATE `creature_template` SET `DamageMultiplier` = '6' WHERE `entry` = 18638;
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.0', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '4124', `MaxMeleeDmg` = '5827' WHERE `entry` = '20650';
-- Cabal Spellbinder 18639,20647 (3/4.05H)(4.66/11.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5554' WHERE `entry` = '20647';
-- Cabal Warlock 18640,20649 (3/4.05H)(4.66/11.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5463' WHERE `entry` = '20649';
-- Cabal Familiar 18641,20643 (1.1/1.485H)(1.894/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.75', `DamageMultiplier` = '7.0', `MinLevelHealth` = '13447', `MaxLevelHealth` = '13447', `MinMeleeDmg` = '1097', `MaxMeleeDmg` = '1592' WHERE `entry` = '20643';
-- Fel Guardhound 18642,20651 (1.45/3H)(2/4D)
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '6.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '1523', `MaxMeleeDmg` = '2152' WHERE `entry` = '20651';
-- Maiden of Discipline 18663,20655 (1.45/1.9574H)(2/9.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '19561', `MaxLevelHealth` = '19561', `MinMeleeDmg` = '3515', `MaxMeleeDmg` = '4966' WHERE `entry` = '20655';
-- Blackheart the Inciter (18667,20637) (18.75/25H)(5.3/10D) - 110750/184500
UPDATE `creature_template` SET `HealthMultiplier` = '15.0068', `DamageMultiplier` = '5.3', `MinLevelHealth` = '110750', `MaxLevelHealth` = '110750', `MinMeleeDmg` = '1911', `MaxMeleeDmg` = '2448' WHERE `entry` = '18667';
UPDATE `creature_template` SET `HealthMultiplier` = '25.0', `DamageMultiplier` = '10.0', `MinLevelHealth` = '184500', `MaxLevelHealth` = '184500', `MinMeleeDmg` = '4144', `MaxMeleeDmg` = '4748' WHERE `entry` = '20637';
-- Murmur (18708,20657) (57.5/77.625H) * 0.4 (5/9D) - 392700/572872
UPDATE `creature_template` SET `DamageMultiplier` = '7.0' WHERE `entry` = 18708;
UPDATE `creature_template` SET `HealthMultiplier` = '86.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '634680', `MaxLevelHealth` = '634680', `MinMeleeDmg` = '5846', `MaxMeleeDmg` = '8267' WHERE `entry` = '20657';
-- Ambassador Hellmaw (18731,20636) (18.25/25H)(6/7D) - 134685/184500
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 18731;
UPDATE `creature_template` SET `HealthMultiplier` = '31.0', `DamageMultiplier` = '14.0', `MinLevelHealth` = '228780', `MaxLevelHealth` = '228780', `MinMeleeDmg` = '5481', `MaxMeleeDmg` = '7750' WHERE `entry` = '20636';
-- Grandmaster Vorpil (18732,20653) (18.75/23.5H)(6/6D) - 110681/138720
UPDATE `creature_template` SET `DamageMultiplier` = '8.0' WHERE `entry` = 18732;
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `DamageMultiplier` = '12.0', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090', `MinMeleeDmg` = '2904', `MaxMeleeDmg` = '4102' WHERE `entry` = '20653';
-- Cabal Ritualist 18794,20645 (3/4.05H)(4.28/10D) - 16227/22635
UPDATE `creature_template` SET `DamageMultiplier` = '6.0' WHERE `entry` = 18794;
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '33534', `MaxLevelHealth` = '43080', `MinMeleeDmg` = '3515', `MaxMeleeDmg` = '5049' WHERE `entry` = '20645';
-- Fel Overseer 18796,20652 (12/16.2H)(10/25D)
UPDATE `creature_template` SET `DamageMultiplier` = '12.0' WHERE `entry` = 18796;
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `DamageMultiplier` = '29.0', `MinLevelHealth` = '146706', `MaxLevelHealth` = '146706', `MinMeleeDmg` = '7322', `MaxMeleeDmg` = '10353' WHERE `entry` = '20652';
-- Tortured Skeleton 18797,20662 (0.4/0.54H)(2/4D)
UPDATE `creature_template` SET `DamageMultiplier` = '3.0' WHERE `entry` = 18797;
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `DamageMultiplier` = '5.0', `MinLevelHealth` = '10479', `MaxLevelHealth` = '10479', `MinMeleeDmg` = '1641', `MaxMeleeDmg` = '2321' WHERE `entry` = '20662';
-- Cabal Fanatic 18830,20644 (3/4.05H)(4.66/11.5D) - 20958/28293
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '5.5', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `MinMeleeDmg` = '1389', `MaxMeleeDmg` = '1964' WHERE `entry` = '18830';
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '15.5', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '4292', `MaxMeleeDmg` = '6069' WHERE `entry` = '20644';
-- Malicious Instructor 18848,20656 (12.25/16.5374H)(6/17D)
UPDATE `creature_template` SET `HealthMultiplier` = '12.25', `MinLevelHealth` = '68465', `MaxLevelHealth` = '68465', `DamageMultiplier` = '10.0' WHERE `entry` = '18848';
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `MinLevelHealth` = '117369', `MaxLevelHealth` = '120624', `DamageMultiplier` = '24.0' WHERE `entry` = '20656';
-- Summoned Cabal Acolyte 19208,20660 (3/4.05H)(4.7/9D)
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5463' WHERE `entry` = '20660';
-- Summoned Cabal Deathsworn 19209,20661 (3/4.05H)(5/11D) - 20283/29083
UPDATE `creature_template` SET `DamageMultiplier` = '6' WHERE `entry` = 19209;
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '15.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '4364', `MaxMeleeDmg` = '6172' WHERE `entry` = '20661';
-- Void Traveler 19226,20664 (0.5/0.625H)(2/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `DamageMultiplier` = '5.0', `MinLevelHealth` = '8383', `MaxLevelHealth` = '8383', `MinMeleeDmg` = '1172', `MaxMeleeDmg` = '1655' WHERE `entry` = '20664';

-- ============================================================================================================================================================================
-- Caverns of Time: Old Hillsbrad Foothills
-- ============================================================================================================================================================================

-- Forest Moss Creeper 2350
-- Vicious Gray Bear 2354
-- Feral Mountain Lion 2385
-- Snapjaw 2408
-- Lordaeron Watchman 17814,20538 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '14.0', `MinLevelHealth` = '33805', `MaxLevelHealth` = '33805', `MinMeleeDmg` = '3383', `MaxMeleeDmg` = '4776' WHERE `entry` = '20538';
-- Lordaeron Sentry 17815,20537 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '13.0', `MinLevelHealth` = '33805', `MaxLevelHealth` = '33805', `MinMeleeDmg` = '3141', `MaxMeleeDmg` = '4435' WHERE `entry` = '20537';
-- Durnholde Sentry 17819,20527 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '35905', `MinMeleeDmg` = '4040', `MaxMeleeDmg` = '5809' WHERE `entry` = '20527';
-- Durnholde Rifleman 17820,20526 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '13.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '35905', `MinMeleeDmg` = '3282', `MaxMeleeDmg` = '4720' WHERE `entry` = '20526';
-- Durnholde Warden 17833,20530 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '13.0', `MinLevelHealth` = '28720', `MaxLevelHealth` = '28720', `MinMeleeDmg` = '3097', `MaxMeleeDmg` = '4376' WHERE `entry` = '20530';
-- Durnholde Tracking Hound 17840,20528 
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `DamageMultiplier` = '11.0', `MinLevelHealth` = '27044', `MaxLevelHealth` = '27044', `MinMeleeDmg` = '1595', `MaxMeleeDmg` = '2252' WHERE `entry` = '20528';
-- Pit Spectator 17846,20543 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '9.0', `MinLevelHealth` = '33805', `MaxLevelHealth` = '33805', `MinMeleeDmg` = '2175', `MaxMeleeDmg` = '3071' WHERE `entry` = '20543';
-- Lieutenant Drake (17848,20535) (12/16.2H)(4.66/9D) - 78504/119556
UPDATE `creature_template` SET `HealthMultiplier` = '12.0', `DamageMultiplier` = '7.0', `MinLevelHealth` = '78504', `MaxLevelHealth` = '78504', `MinMeleeDmg` = '1615', `MaxMeleeDmg` = '2276' WHERE `entry` = '17848';
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980', `MinMeleeDmg` = '4176', `MaxMeleeDmg` = '5905' WHERE `entry` = '20535';
-- Durnholde Veteran 17860,20529 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '35905', `MaxLevelHealth` = '35905', `MinMeleeDmg` = '3851', `MaxMeleeDmg` = '5446' WHERE `entry` = '20529';
-- Captain Skarloc (17862,20521) (12/16.2H)(1.5/13D) - 60108/95629
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `DamageMultiplier` = '19.0', `MinLevelHealth` = '123963', `MaxLevelHealth` = '123963', `MinMeleeDmg` = '4598', `MaxMeleeDmg` = '6495' WHERE `entry` = '20521';
-- Thrall 17876,20548 
-- Tarren Mill Guardsman 18092,20545 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '35905', `MaxLevelHealth` = '36900', `MinMeleeDmg` = '3851', `MaxMeleeDmg` = '5536' WHERE `entry` = '20545';
-- Tarren Mill Protector 18093,20547 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '10.0', `MinLevelHealth` = '28720', `MaxLevelHealth` = '29515', `MinMeleeDmg` = '3573', `MaxMeleeDmg` = '5128' WHERE `entry` = '20547';
-- Tarren Mill Lookout 18094,20546 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '28720', `MaxLevelHealth` = '29515', `MinMeleeDmg` = '3573', `MaxMeleeDmg` = '5128' WHERE `entry` = '20546';
-- Epoch Hunter (18096,20531) (15/20.25H)(6.7/20D) - 98130/149445
UPDATE `creature_template` SET `HealthMultiplier` = '32.0', `DamageMultiplier` = '28.0', `MinLevelHealth` = '236160', `MaxLevelHealth` = '236160', `MinMeleeDmg` = '7308', `MaxMeleeDmg` = '10334' WHERE `entry` = '20531';
-- Infinite Slayer 18170,20534 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '35905', `MaxLevelHealth` = '36900', `MinMeleeDmg` = '3915', `MaxMeleeDmg` = '5536' WHERE `entry` = '20534';
-- Infinite Defiler 18171,20532 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '29515', `MaxLevelHealth` = '29515', `MinMeleeDmg` = '3630', `MaxMeleeDmg` = '5128' WHERE `entry` = '20532';
-- Infinite Saboteur 18172,20533 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '28720', `MaxLevelHealth` = '29515', `MinMeleeDmg` = '3630', `MaxMeleeDmg` = '5128' WHERE `entry` = '20533';
-- Orc Prisoner 18598,20541 
UPDATE `creature_template` SET `HealthMultiplier` = '2.25', `DamageMultiplier` = '0.5', `MinLevelHealth` = '15212', `MaxLevelHealth` = '15212', `MinMeleeDmg` = '121', `MaxMeleeDmg` = '171' WHERE `entry` = '20541';
-- Tarren Mill Peasant 18644
-- Tarren Mill Horsehand 18646, 
-- Innkeeper Monica 18649, 
-- Young Blanchy 18651, 
-- Jay Lemieux 18655, 
-- Julie Honeywell 18656, 
-- Tarren Mill Fisherman 18657, 
-- Aged Dalaran Wizard 18664, 
-- Dalaran Sorceress 18666, 
-- Thomas Yance 18672, 
-- Pit Announcer 18673,20542 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '8.0', `MinLevelHealth` = '33805', `MaxLevelHealth` = '33805', `MinMeleeDmg` = '1933', `MaxMeleeDmg` = '2729' WHERE `entry` = '20542';
-- Erozion 18723
-- Brazen 18725
-- Durnholde Armorer 18764,20523 
UPDATE `creature_template` SET `HealthMultiplier` = '2.25', `DamageMultiplier` = '9.0', `MinLevelHealth` = '15212', `MaxLevelHealth` = '15212', `MinMeleeDmg` = '2175', `MaxMeleeDmg` = '3071' WHERE `entry` = '20523';
-- Durnholde Cook 18765,20524 
UPDATE `creature_template` SET `HealthMultiplier` = '2.25', `DamageMultiplier` = '9.0', `MinLevelHealth` = '15212', `MaxLevelHealth` = '15212', `MinMeleeDmg` = '2175', `MaxMeleeDmg` = '3071' WHERE `entry` = '20524';
-- Taretha 18887
-- Durnholde Mage 18934,20525 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '9.0', `MinLevelHealth` = '28720', `MaxLevelHealth` = '28720', `MinMeleeDmg` = '2144', `MaxMeleeDmg` = '3030' WHERE `entry` = '20525';
-- Image of Erozion 19438
-- Hal McAllister 20342
-- Nat Pagle 20344
-- Commander Mograine 20345
-- Isillien 20346
-- Abbendis 20347
-- Fairbanks 20348
-- Tirion Fordring 20349
-- Kel'Thuzad 20350
-- Captain Sanders 20351
-- Arcanist Doan 20352
-- Helcular 20353
-- Nathanos Marris 20354
-- Stalvan Mistmantle 20355
-- Sally Whitemane 20357
-- Renault Mograine 20358
-- Little Jimmy Vishas 20359
-- Herod the Bully 20360
-- Taelan 20361
-- Caretaker Smithers 20363
-- Bartolo Ginsetti 20365
-- Farmer Kent 20368
-- Phin Odelic 20370
-- Jonathan Revah 20372
-- Magistrate Henry Maleb 20373
-- Jerry Carter 20376
-- Barkeep Kelly 20377
-- Chef Jessen 20378
-- Bilger the Straight-laced 20379
-- Raleigh the True 20380
-- Captured Critter 20396
-- Reanimated Critter 20398
-- Captain Edward Hanes 20400
-- Frances Lin 20401
-- Zixil 20419
-- Overwatch Mark 0 20420
-- Kirin Tor Mage 20422
-- Hillsbrad Peasant 20424
-- Hillsbrad Citizen 20426
-- Beggar 20432
-- Hillsbrad Farmer 20433
-- Natasha Morris 20441
-- Victor 21341
-- Alex 21342
-- Harvey 21343
-- Phil 21344
-- Hugh 21345
-- Durnholde Lookout 22128,22129 
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '36900', `MaxLevelHealth` = '36900', `MinMeleeDmg` = '4176', `MaxMeleeDmg` = '5905' WHERE `entry` = '22129';
-- Durnholde Reinforcement 22398,22399 
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `DamageMultiplier` = '12.0', `MinLevelHealth` = '48902', `MaxLevelHealth` = '48902', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4357' WHERE `entry` = '22399';

-- ============================================================================================================================================================================
-- Caverns of Time: The Black Morass
-- ============================================================================================================================================================================

-- Shadow Council Enforcer 17023
-- Infinite Assassin 17835,20740+ (2.3/7D)
UPDATE `creature_template` SET `DamageMultiplier` = '9.0' WHERE `entry` = '20740';
-- Infinite Assassin 21137,22164+ (2.3/7D)
UPDATE `creature_template` SET `DamageMultiplier` = '9.0' WHERE `entry` = '22164';
-- Rift Lord 17839,20744+ (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `DamageMultiplier` = '4.0' WHERE `entry` = '17839';
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '14.0' WHERE `entry` = '20744';
-- Rift Lord 21140,22172+ (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '9', `DamageMultiplier` = '4.0' WHERE `entry` = '21140';
UPDATE `creature_template` SET `HealthMultiplier` = '12', `DamageMultiplier` = '14.0' WHERE `entry` = '22172';
-- Chrono Lord Deja (17879,20738) (15/20H)(8.6/12D) - 88545/120000
UPDATE `creature_template` SET `DamageMultiplier` = '17.0' WHERE `entry` = '20738';
-- Infinite Chrono-Lord (21697,21712) (15/20H)(6.013/15D) - 88545/118060
UPDATE `creature_template` SET `DamageMultiplier` = '17.0' WHERE `entry` = '21712';
-- Temporus (17880,20745) (15/20H)(10/12D) - 110700/151760
UPDATE `creature_template` SET `DamageMultiplier` = '16.0' WHERE `entry` = '20745';
-- Infinite Timereaver 21698,22167+ (10/12D)
UPDATE `creature_template` SET `DamageMultiplier` = '16.0' WHERE `entry` = '22167';
-- Aeonus (17881,20737) (20/27H)(13/15D) - 147600/199260
UPDATE `creature_template` SET `DamageMultiplier` = '19.0' WHERE `entry` = '20737';
-- Infinite Chronomancer 17892,20741+ (2.5/8D)
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = '20741';
-- Infinite Chronomancer 21136,22165+ (2.5/8D)
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = '22165';
-- Time Keeper 17918,20746 
-- Darkwater Crocolisk 17952,22163+ (1.5/5D)
UPDATE `creature_template` SET `DamageMultiplier` = '6.0' WHERE `entry` = '22163';
-- Sable Jaguar 18982,22173+ (2/3D)
UPDATE `creature_template` SET `DamageMultiplier` = '4.0' WHERE `entry` = '22173';
-- Blackfang Tarantula 18983,22162+ (1.5/2D)
UPDATE `creature_template` SET `DamageMultiplier` = '3.0' WHERE `entry` = '22162';
-- Infinite Executioner 18994,20742+ (2.5/8D)
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '18994';
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = '20742';
-- Infinite Executioner 21138,22166+ (2.5/8D)
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '21138';
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = '22166';
-- Infinite Vanquisher 18995,20743+ (2/5D)
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '18995';
UPDATE `creature_template` SET `DamageMultiplier` = '7.0' WHERE `entry` = '20743';
-- Infinite Vanquisher 21139,22168+ (2/5D)
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = '21139';
UPDATE `creature_template` SET `DamageMultiplier` = '7.0' WHERE `entry` = '22168';
-- Sa'at 20201 
-- Rift Keeper 21104,22170+ (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `DamageMultiplier` = '4.0' WHERE `entry` = '21104';
UPDATE `creature_template` SET `HealthMultiplier` = '12.0', `DamageMultiplier` = '14.0' WHERE `entry` = '22170';
-- Rift Keeper 21148,22171+ (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `DamageMultiplier` = '4.0' WHERE `entry` = '21148';
UPDATE `creature_template` SET `HealthMultiplier` = '12.0', `DamageMultiplier` = '14.0' WHERE `entry` = '22171';
-- Infinite Whelp 21818,22169+ (0.56/2D)
UPDATE `creature_template` SET `DamageMultiplier` = '3.0' WHERE `entry` = '22169';

-- ============================================================================================================================================================================
-- Coilfang Reservoir: The Slave Pens
-- ============================================================================================================================================================================

-- Bogstrok 17816,19884 
UPDATE `creature_template` SET `HealthMultiplier` = '4.25', `DamageMultiplier` = '14.0', `MinLevelHealth` = '29690', `MaxLevelHealth` = '29690', `MinMeleeDmg` = '3535', `MaxMeleeDmg` = '4998' WHERE `entry` = '19884';
-- Greater Bogstrok 17817,19892 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '25.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '6312', `MaxMeleeDmg` = '8925' WHERE `entry` = '19892';
-- Naturalist Bite 17893,22938 
-- Coilfang Observer 17938,19888 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25150', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4635' WHERE `entry` = '19888';
-- Coilfang Technician 17940,19891 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4713' WHERE `entry` = '19891';
-- Mennu the Betrayer (17941,19893) (17/22.95H)(5.8/13D) - 77724/132040
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `DamageMultiplier` = '21.0', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090', `MinMeleeDmg` = '3049', `MaxMeleeDmg` = '4307' WHERE `entry` = '19893';
-- Quagmirran (17942,19894) (18/24.3H)(8/16D) - 102870/179334
UPDATE `creature_template` SET `HealthMultiplier` = '34.0', `DamageMultiplier` = '20.0', `MinLevelHealth` = '250920', `MaxLevelHealth` = '250920', `MinMeleeDmg` = '8221', `MaxMeleeDmg` = '11626' WHERE `entry` = '19894';
-- Coilfang Champion 17957,19885 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '19885';
-- Coilfang Defender 17958,19886 (4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3594', `MaxMeleeDmg` = '5083' WHERE `entry` = '19886';
-- Coilfang Slavehandler 17959,19889 
UPDATE `creature_template` SET `HealthMultiplier` = '4.75', `DamageMultiplier` = '15.0', `MinLevelHealth` = '33183', `MaxLevelHealth` = '34109' WHERE `entry` = '19889';
-- Coilfang Soothsayer 17960,19890 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '22005', `MaxLevelHealth` = '22005', `MinMeleeDmg` = '2744', `MaxMeleeDmg` = '3980' WHERE `entry` = '19890';
-- Coilfang Enchantress 17961,19887 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25150', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4635' WHERE `entry` = '19887';
-- Coilfang Collaborator 17962,19903 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '2121', `MaxMeleeDmg` = '2999' WHERE `entry` = '19903';
-- Wastewalker Slave 17963,19902 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '9.0', `MinLevelHealth` = '12225', `MaxLevelHealth` = '12567', `MinMeleeDmg` = '1764', `MaxMeleeDmg` = '2671' WHERE `entry` = '19902';
-- Wastewalker Worker 17964,19904 
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '11.0', `MinLevelHealth` = '17465', `MaxLevelHealth` = '17952', `MinMeleeDmg` = '1666', `MaxMeleeDmg` = '2396' WHERE `entry` = '19904';
-- Rokmar the Crackler (17991,19895) (17/22.95H)(6.5/14D) - 97155/169371
UPDATE `creature_template` SET `HealthMultiplier` = '31.0', `DamageMultiplier` = '18.0', `MinLevelHealth` = '228780', `MaxLevelHealth` = '228780', `MinMeleeDmg` = '5481', `MaxMeleeDmg` = '7750' WHERE `entry` = '19895';
-- Wastewalker Captive 18206,19901 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '9.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '2272', `MaxMeleeDmg` = '3213' WHERE `entry` = '19901';
-- Coilfang Scale-Healer 21126,21842 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25150', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4635' WHERE `entry` = '21842';
-- Coilfang Tempest 21127,21843 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3535', `MaxMeleeDmg` = '4998' WHERE `entry` = '21843';
-- Coilfang Ray 21128,21841 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '9.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '2272', `MaxMeleeDmg` = '3213' WHERE `entry` = '21841';
-- https://wow.gamepedia.com/index.php?title=Ahune&oldid=1606463 - 230000
-- Ahune (25740,26338) (23/30.25H)(8/9D) - 150000/230000
UPDATE `creature_template` SET `HealthMultiplier` = '38.0', `DamageMultiplier` = '18.0', `MinLevelHealth` = '288344', `MaxLevelHealth` = '288344', `MinMeleeDmg` = '5094', `MaxMeleeDmg` = '7230' WHERE `entry` = '26338';
-- Ahunite Hailstone 25755,26342 
-- Ahunite Coldwave 25756,26340 
-- Ahunite Frostwind 25757,26341
-- Frozen Core (25865,26339) (40/52H) - 303520/394576

-- ============================================================================================================================================================================
-- Coilfang Reservoir: The Underbog
-- ============================================================================================================================================================================

-- Bog Giant 17723,20164+ (6.6/8.91H)(7.21/20D) - 36478/62245 - Spells Scale with DamageMultiplier
UPDATE `creature_template` SET `HealthMultiplier` = '8.91', `DamageMultiplier` = '26.0', `MinLevelHealth` = '62245', `MaxLevelHealth` = '62245', `MinMeleeDmg` = '6565', `MaxMeleeDmg` = '9282' WHERE `entry` = '20164';
-- Underbat 17724,20185 (2.9/3.915H)(4/20D) - 15489/27350
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '4.0', `MinLevelHealth` = '15489', `MaxLevelHealth` = '16028', `MinMeleeDmg` = '689', `MaxMeleeDmg` = '1010' WHERE `entry` = '17724';
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '18.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '5807', `MaxMeleeDmg` = '8211' WHERE `entry` = '20185';
-- Underbog Lurker 17725,20188 (2.9/3.915H)(4/20D) - 15489/26469
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '4.0', `MinLevelHealth` = '15489', `MaxLevelHealth` = '16028', `MinMeleeDmg` = '862', `MaxMeleeDmg` = '1263' WHERE `entry` = '17725';
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '23.0', `MinLevelHealth` = '30424', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '6947', `MaxMeleeDmg` = '10264' WHERE `entry` = '20188';
-- Wrathfin Myrmidon 17726,20191 (2.9/3.915H)(4/17D) - 16574/28114
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '20.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '5648', `MaxMeleeDmg` = '7988' WHERE `entry` = '20191';
-- Wrathfin Sentry 17727,20192 (2.9/3.915H)(4/17D) - 16574/28114
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '20.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '5648', `MaxMeleeDmg` = '7988' WHERE `entry` = '20192';
-- Murkblood Tribesman 17728,20181 (2.9/3.915H)(4/8D) - 16028/27350
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '11.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4357' WHERE `entry` = '20181';
-- Murkblood Spearman 17729,20180 (2.9/3.915H)(4/5D) - 12824/21881
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '8.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '1640', `MaxMeleeDmg` = '2356' WHERE `entry` = '20180';
-- Murkblood Healer 17730,20177 (2.9/3.915H)(4/5D) - 12824/22488
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '7.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '1640', `MaxMeleeDmg` = '2356' WHERE `entry` = '20177';
-- Fen Ray 17731,20173 (2.9/3.915h)(3.6/9D) - 16574/28114
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3337', `MaxMeleeDmg` = '4720' WHERE `entry` = '20173';
-- Lykul Wasp 17732,20175 (2.9/2.9H)(3.6/9D) - 16028/20259
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '4.5', `MinLevelHealth` = '16028', `MaxLevelHealth` = '16028', `MinMeleeDmg` = '815', `MaxMeleeDmg` = '1137' WHERE `entry` = '17732';
UPDATE `creature_template` SET `HealthMultiplier` = '3.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '24451', `MaxLevelHealth` = '24451', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4284' WHERE `entry` = '20175';
-- Underbog Lord 17734,20187 (8/10H)(8/20D) - 45720/71810
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `DamageMultiplier` = '11.0', `MinLevelHealth` = '45720', `MaxLevelHealth` = '45720', `MinMeleeDmg` = '2622', `MaxMeleeDmg` = '3667' WHERE `entry` = '17734';
UPDATE `creature_template` SET `HealthMultiplier` = '14.0', `DamageMultiplier` = '23.0', `MinLevelHealth` = '100534', `MaxLevelHealth` = '100534', `MinMeleeDmg` = '8022', `MaxMeleeDmg` = '11346' WHERE `entry` = '20187';
-- Wrathfin Warrior 17735,20193 (2.9/3.915H)(4/17D) - 16574/28114
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '20.0', `MinLevelHealth` = '32314', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '5648', `MaxMeleeDmg` = '7988' WHERE `entry` = '20193';
-- Hungarfen (17770,20169) (11/14.85H)(10/20D) - 65054/110054
-- Murkblood Oracle 17771,20179 (2.9/3.915H)(4/4D) - 12824/21881
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '6.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '2020' WHERE `entry` = '20179';
-- Swamplord Musel'ek (17826,20183) (11.25/15.1875H)(9/20D) - 53224/89224
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `DamageMultiplier` = '23.0', `MinLevelHealth` = '123963', `MaxLevelHealth` = '123963', `MinMeleeDmg` = '6098', `MaxMeleeDmg` = '8615' WHERE `entry` = '20183';
-- Claw (17827,20165) (9/12.15H)(1/2.5D) - 53226/89667
UPDATE `creature_template` SET `HealthMultiplier` = '18.0', `DamageMultiplier` = '2.5', `MinLevelHealth` = '132840', `MaxLevelHealth` = '132840', `MinMeleeDmg` = '2610', `MaxMeleeDmg` = '3691' WHERE `entry` = '20165';
-- Underbog Shambler 17871,20190 (2.9/3.9H)(3.6/8D) - 12395/21797
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '11.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25150', `MinMeleeDmg` = '3515', `MaxMeleeDmg` = '4966' WHERE `entry` = '20190';
-- The Black Stalker (17882,20184) (16/21.6H)(9.6/10D) - 75696/127505
UPDATE `creature_template` SET `HealthMultiplier` = '32.0', `DamageMultiplier` = '18.0', `MinLevelHealth` = '188896', `MaxLevelHealth` = '188896', `MinMeleeDmg` = '4235', `MaxMeleeDmg` = '5982' WHERE `entry` = '20184';
-- Underbog Mushroom 17990,20189 (6/8.1H)(1/2.5D) - 35484/59778
-- Ghaz'an (18105,20168) (10/13.5H)(6/20D) - 59140/99630
UPDATE `creature_template` SET `HealthMultiplier` = '20.0', `DamageMultiplier` = '29.0', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600', `MinMeleeDmg` = '6055', `MaxMeleeDmg` = '8562' WHERE `entry` = '20168';
-- Overseer Tidewrath (18107) (12H/4.4D) - 250000
-- Lykul Stinger 19632,20174 (2.9/5.5H)(3.6/9D) - 16574/39496
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '4.5', `MinLevelHealth` = '16574', `MaxLevelHealth` = '16574', `MinMeleeDmg` = '858', `MaxMeleeDmg` = '1200' WHERE `entry` = '19632';
UPDATE `creature_template` SET `HealthMultiplier` = '6.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '46676', `MaxLevelHealth` = '46676', `MinMeleeDmg` = '3081', `MaxMeleeDmg` = '4357' WHERE `entry` = '20174';
-- Spore Strider 22299,22300+ (1/1.25H)(1/2.5D) - 5589/6986
UPDATE `creature_template` SET `HealthMultiplier` = '1.00', `MinLevelHealth` = '5589', `MaxLevelHealth` = '5589' WHERE `entry` = '22300';

-- ============================================================================================================================================================================
-- Coilfang Reservoir: The Steamvault
-- ============================================================================================================================================================================

-- Coilfang Engineer 17721,20620 (2.9/3.915H)(4.66/11.5D) - 20259/27350
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '5.5', `MinLevelHealth` = '20259', `MaxLevelHealth` = '20259', `MinMeleeDmg` = '1389', `MaxMeleeDmg` = '1964' WHERE `entry` = '17721';
UPDATE `creature_template` SET `HealthMultiplier` = '5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '20620';
-- Coilfang Sorceress 17722,20625 (4/5.4H)(4.66/11.5D) - 22356/30181
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5463' WHERE `entry` = '20625';
-- Mekgineer Steamrigger (17796,20630) (14/17H)(8/10D) - 103320/125460
UPDATE `creature_template` SET `HealthMultiplier` = '24.0', `DamageMultiplier` = '19.0', `MinLevelHealth` = '177120', `MaxLevelHealth` = '177120', `MinMeleeDmg` = '4959', `MaxMeleeDmg` = '7012' WHERE `entry` = '20630';
-- Hydromancer Thespia (17797,20629) (17/24H)(6.5/9D) - 100531/141672
UPDATE `creature_template` SET `HealthMultiplier` = '34.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '200702', `MaxLevelHealth` = '200702', `MinMeleeDmg` = '3872', `MaxMeleeDmg` = '5470' WHERE `entry` = '20629';
-- Warlord Kalithresh (17798,20633) (20/32H)(6.5/12D) - 147600/236160
UPDATE `creature_template` SET `HealthMultiplier` = '42.0', `DamageMultiplier` = '21.0', `MinLevelHealth` = '309960', `MaxLevelHealth` = '309960', `MinMeleeDmg` = '5481', `MaxMeleeDmg` = '7750' WHERE `entry` = '20633';
-- Dreghood Slave 17799,20628 (2.9/3.915H)(4.66/7D) - 20259/27350
UPDATE `creature_template` SET `HealthMultiplier` = '5', `DamageMultiplier` = '10.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '1515', `MaxMeleeDmg` = '2142' WHERE `entry` = '20628';
-- Coilfang Myrmidon 17800,20621 (2.9/3.915H)(4.66/11.5D) - 20259/27350
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '14.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '4166', `MaxMeleeDmg` = '5891' WHERE `entry` = '20621';
-- Coilfang Siren 17801,20623 (2.9/3.915H)(4.66/11.5D) - 16208/21881
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5463' WHERE `entry` = '20623';
-- Coilfang Warrior 17802,20626 (2.9/3.915H)(4.66/11.5D) - 20259/27350
UPDATE `creature_template` SET `HealthMultiplier` = '5', `DamageMultiplier` = '16.5', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '4166', `MaxMeleeDmg` = '5891' WHERE `entry` = '20626';
-- Coilfang Oracle 17803,20622 (2.9/3.915H)(4.66/11.5D) - 16208/21881
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '16.5', `MinLevelHealth` = '33534', `MaxLevelHealth` = '33534', `MinMeleeDmg` = '3866', `MaxMeleeDmg` = '5463' WHERE `entry` = '20622';
-- Coilfang Slavemaster 17805,20624 (2.9/3.915H)(4.66/11.5D) - 20259/27350
UPDATE `creature_template` SET `HealthMultiplier` = '5', `DamageMultiplier` = '16.5', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `MinMeleeDmg` = '4166', `MaxMeleeDmg` = '5891' WHERE `entry` = '20624';
-- Coilfang Water Elemental 17917,20627 (3/4.05H)(1.5/12D) - 16767/22635 - SPELL_SCHOOL_FROST
UPDATE `creature_template` SET `HealthMultiplier` = '4.05', `DamageMultiplier` = '14.0', `MinLevelHealth` = '22635', `MaxLevelHealth` = '22635', `MinMeleeDmg` = '4101', `MaxMeleeDmg` = '5794' WHERE `entry` = '20627';
-- Steamrigger Mechanic 17951,20632 (1/2.1H)(1.3/5D) - 5589/11737 - https://www.wowhead.com/npc=17951/steamrigger-mechanic#comments:id=122636
UPDATE `creature_template` SET `HealthMultiplier` = '2.3', `DamageMultiplier` = '5.0', `MinLevelHealth` = '12855', `MaxLevelHealth` = '12855', `MinMeleeDmg` = '1640', `MaxMeleeDmg` = '2317' WHERE `entry` = '20632';
-- Naga Distiller 17954,20631 (1.5/2.025H) - 10479/14147
UPDATE `creature_template` SET `HealthMultiplier` = '1.5', `MinLevelHealth` = '10479', `MaxLevelHealth` = '10479' WHERE `entry` = '17954';
UPDATE `creature_template` SET `HealthMultiplier` = '3.25', `MinLevelHealth` = '22705', `MaxLevelHealth` = '22705' WHERE `entry` = '20631';
-- Coilfang Leper 21338,21915 (1/1.1H)(1.75/3.5D) - 5589/6148
UPDATE `creature_template` SET `HealthMultiplier` = '2.3', `DamageMultiplier` = '4.0', `MinLevelHealth` = '12855', `MaxLevelHealth` = '12855', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '1986' WHERE `entry` = '21915';
-- Bog Overlord 21694,21914 (13/30D) - (6.5/8.775H)(13/30D) - 45409/61302
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `DamageMultiplier` = '33.0', `MinLevelHealth` = '90818', `MaxLevelHealth` = '90818', `MinMeleeDmg` = '9089', `MaxMeleeDmg` = '12852' WHERE `entry` = '21914';
-- Tidal Surger 21695,21917 (6/8.1H)(5.5/12D) 33534/45271 - SPELL_SCHOOL_FROST
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `DamageMultiplier` = '14.0', `MinLevelHealth` = '61479', `MaxLevelHealth` = '61479', `MinMeleeDmg` = '4218', `MaxMeleeDmg` = '5959' WHERE `entry` = '21917';
-- Steam Surger 21696,21916 (0.5/0.675H)(1.5/4D) - 2795/3773 - SPELL_SCHOOL_FROST
UPDATE `creature_template` SET `HealthMultiplier` = '0.5', `DamageMultiplier` = '1.5', `MinLevelHealth` = '2795', `MaxLevelHealth` = '2795', `MinMeleeDmg` = '1054', `MaxMeleeDmg` = '1490' WHERE `entry` = '21696';
UPDATE `creature_template` SET `HealthMultiplier` = '1.0', `DamageMultiplier` = '4.0', `MinLevelHealth` = '5589', `MaxLevelHealth` = '5589', `MinMeleeDmg` = '2812', `MaxMeleeDmg` = '3973' WHERE `entry` = '21916';
-- Second Fragment Guardian 22891(3H)(3D) - 20958

-- ============================================================================================================================================================================
-- Hellfire Citadel: Hellfire Ramparts - Initial Testing In Progress Based on Base Values Then Will Pre-Nerf Tune As Required 
-- ============================================================================================================================================================================

-- Bonechewer Hungerer 17259,18053 (4/21D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '24.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '3636', `MaxMeleeDmg` = '5228' WHERE `entry` = '18053';
-- Bonechewer Ravener 17264,18054 (4/16D) - https://www.wowhead.com/npc=17264/bonechewer-ravener#comments:id=49385 -> 30D
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '19.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '2878', `MaxMeleeDmg` = '4139' WHERE `entry` = '18054';
-- Bleeding Hollow Darkcaster 17269,18049 (4/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '16.0', `MinLevelHealth` = '22005', `MaxLevelHealth` = '22621', `MinMeleeDmg` = '3135', `MaxMeleeDmg` = '4749' WHERE `entry` = '18049';
-- Bleeding Hollow Archer 17270,18048 (4/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '17.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25848', `MinMeleeDmg` = '2390', `MaxMeleeDmg` = '3434' WHERE `entry` = '18048';
-- Bonechewer Destroyer 17271,18052 (4.33/16D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '19.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '32314', `MinMeleeDmg` = '4797', `MaxMeleeDmg` = '6898' WHERE `entry` = '18052';
-- Shattered Hand Warhound 17280,18059 (1/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '7.5', `MinLevelHealth` = '13522', `MaxLevelHealth` = '13522', `MinMeleeDmg` = '1359', `MaxMeleeDmg` = '1919' WHERE `entry` = '18059';
-- Bonechewer Ripper 17281,18055 (4/21D)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `DamageMultiplier` = '24.0', `MinLevelHealth` = '71810', `MaxLevelHealth` = '71810', `MinMeleeDmg` = '3697', `MaxMeleeDmg` = '5228' WHERE `entry` = '18055';
-- Watchkeeper Gargolmar (17306,18436) (6.25/14H)(6/20D) - 33381/103320
UPDATE `creature_template` SET `HealthMultiplier` = '16.0', `DamageMultiplier` = '23.0', `MinLevelHealth` = '118080', `MaxLevelHealth` = '118080', `MinMeleeDmg` = '3602', `MaxMeleeDmg` = '5093' WHERE `entry` = '18436';
-- Vazruden the Herald 17307,18435 (1/1D)
UPDATE `creature_template` SET `HealthMultiplier` = '14.0', `DamageMultiplier` = '2.0', `MinLevelHealth` = '103320', `MaxLevelHealth` = '103320', `MinMeleeDmg` = '522', `MaxMeleeDmg` = '738' WHERE `entry` = '18435';
-- Omor the Unscarred (17308,18433) (14/14H)(7/8D) - 59836/82642
UPDATE `creature_template` SET `HealthMultiplier` = '18.0', `DamageMultiplier` = '14.0', `MinLevelHealth` = '106254', `MaxLevelHealth` = '106254', `MinMeleeDmg` = '4356', `MaxMeleeDmg` = '6153' WHERE `entry` = '18433';
-- Hellfire Watcher 17309,18058 (4.3/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '13.0', `MinLevelHealth` = '28398', `MaxLevelHealth` = '28398', `MinMeleeDmg` = '1575', `MaxMeleeDmg` = '2283' WHERE `entry` = '18058';
-- Bonechewer Beastmaster 17455,18051 (4/13D)
UPDATE `creature_template` SET `DamageMultiplier` = '16.0' WHERE `entry` = '18051';
-- Bleeding Hollow Scryer 17478,18050 (8/11D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '17115', `MaxLevelHealth` = '17594', `MinMeleeDmg` = '1646', `MaxMeleeDmg` = '2493' WHERE `entry` = '18050';
-- Hellfire Sentry 17517,18057 (6/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `DamageMultiplier` = '17.0', `MinLevelHealth` = '40566', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '2465', `MaxMeleeDmg` = '3641' WHERE `entry` = '18057';
-- Nazan (17536,18432) (6.25/12H)(4.376/12D) - 33381/88560
UPDATE `creature_template` SET `HealthMultiplier` = '15.0', `DamageMultiplier` = '12.0', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700', `MinMeleeDmg` = '3915', `MaxMeleeDmg` = '5536' WHERE `entry` = '18432';
-- Vazruden (17537,18434) (3.6/8.5H)(5.3/8D) - 19228/62730
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `DamageMultiplier` = '8.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `MinMeleeDmg` = '3132', `MaxMeleeDmg` = '4429' WHERE `entry` = '18434';
-- Fiendish Hound 17540,18056 (1.43/6D)

-- ============================================================================================================================================================================
-- Hellfire Citadel: The Blood Furnace
-- ============================================================================================================================================================================

-- Shadowmoon Channeler 17653,18620 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '22621', `MaxLevelHealth` = '22621', `MinMeleeDmg` = '3066', `MaxMeleeDmg` = '4452' WHERE `entry` = '18620';
-- Laughing Skull Enforcer 17370,18608 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '17.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '4292', `MaxMeleeDmg` = '6069' WHERE `entry` = '18608';
-- Shadowmoon Warlock 17371,18619 (2.9/3.915H)(4/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '17.0', `MinLevelHealth` = '24450', `MaxLevelHealth` = '24450', `MinMeleeDmg` = '3331', `MaxMeleeDmg` = '5045' WHERE `entry` = '18619';
-- Keli'dan the Breaker (17377,18607) (9/20H)(6/14D) - 34830/103302
UPDATE `creature_template` SET `HealthMultiplier` = '24.0', `DamageMultiplier` = '18.0', `MinLevelHealth` = '123984', `MaxLevelHealth` = '123984', `MinMeleeDmg` = '3834', `MaxMeleeDmg` = '5571' WHERE `entry` = '18607';
-- Broggok (17380,18601) (8/16H)(4.7/15D) - 30960/82656
UPDATE `creature_template` SET `HealthMultiplier` = '27.0', `DamageMultiplier` = '19.0', `MinLevelHealth` = '139482', `MaxLevelHealth` = '139482', `MinMeleeDmg` = '4899', `MaxMeleeDmg` = '7119' WHERE `entry` = '18601';
-- The Maker (17381,18621) (7.25/14H)(6/10D) - 38722/100534
UPDATE `creature_template` SET `HealthMultiplier` = '17.0', `DamageMultiplier` = '14.0', `MinLevelHealth` = '122077', `MaxLevelHealth` = '122077', `MinMeleeDmg` = '4492', `MaxMeleeDmg` = '6354' WHERE `entry` = '18621';
-- Shadowmoon Summoner 17395,18617 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '16.0', `MinLevelHealth` = '22005', `MaxLevelHealth` = '22005', `MinMeleeDmg` = '3135', `MaxMeleeDmg` = '4549' WHERE `entry` = '18617';
-- Shadowmoon Adept 17397,18615 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '17.0', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25150', `MinMeleeDmg` = '3984', `MaxMeleeDmg` = '5628' WHERE `entry` = '18615';
-- Nascent Fel Orc 17398,18612 (1.35/3.6H)(1.7/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.6', `DamageMultiplier` = '8', `MinLevelHealth` = '25150', `MaxLevelHealth` = '25852', `MinMeleeDmg` = '1919', `MaxMeleeDmg` = '2759' WHERE `entry` = '18612';
-- Seductress 17399,18614 
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '9.0', `MinLevelHealth` = '11178', `MaxLevelHealth` = '11178', `MinMeleeDmg` = '2109', `MaxMeleeDmg` = '2980' WHERE `entry` = '18614';
-- Felguard Annihilator 17400,18604+ (2.9/3.915H)(4/10D) - 15489/28114
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `MinLevelHealth` = '43086', `MaxLevelHealth` = '43086', `DamageMultiplier` = '22.0' WHERE `entry` = '18604'; -- real prenerf even higher 25-30 https://www.wowhead.com/npc=17400/felguard-annihilator#comments:id=37842
-- Felhound Manastalker 17401,18605 
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '10.0', `MinLevelHealth` = '10818', `MaxLevelHealth` = '11178', `MinMeleeDmg` = '2241', `MaxMeleeDmg` = '3311' WHERE `entry` = '18605';
-- Shadowmoon Technician 17414,18618 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '17.0', `MinLevelHealth` = '22005', `MaxLevelHealth` = '22621', `MinMeleeDmg` = '3331', `MaxMeleeDmg` = '5045' WHERE `entry` = '18618';
-- Fel Orc Neophyte 17429,18603 (2/2.7H)(3.1/8D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.7', `DamageMultiplier` = '8', `MinLevelHealth` = '19389', `MaxLevelHealth` = '19389', `MinMeleeDmg` = '1585', `MaxMeleeDmg` = '2242' WHERE `entry` = '18603';
-- Hellfire Imp - 17477,18606 
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '7.5', `MinLevelHealth` = '11178', `MaxLevelHealth` = '11178', `MinMeleeDmg` = '1757', `MaxMeleeDmg` = '2483' WHERE `entry` = '18606';
-- Laughing Skull Rogue 17491,18610 (1.45/1.9575H)(4/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '16.0', `MinLevelHealth` = '15474', `MaxLevelHealth` = '15474', `MinMeleeDmg` = '2040', `MaxMeleeDmg` = '2825' WHERE `entry` = '18610';
-- Laughing Skull Warden 17624,18611 
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `MinMeleeDmg` = '3851', `MaxMeleeDmg` = '5446' WHERE `entry` = '18611';
-- Laughing Skull Legionaire 17626,18609 
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '18609';
-- Felguard Brute 18894,21645 (2.9/3.915H)(4/10D) - 14958/27350
UPDATE `creature_template` SET `HealthMultiplier` = '6', `DamageMultiplier` = '18.0', `MinLevelHealth` = '41916', `MaxLevelHealth` = '41916', `MinMeleeDmg` = '3661', `MaxMeleeDmg` = '5177' WHERE `entry` = '21645';
-- Hellfire Familiar 19016,21646 
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '2.5', `MinLevelHealth` = '11178', `MaxLevelHealth` = '11178', `MinMeleeDmg` = '586', `MaxMeleeDmg` = '828' WHERE `entry` = '21646';

-- ============================================================================================================================================================================
-- Hellfire Citadel: The Shattered Halls
-- ============================================================================================================================================================================

-- Rabid Warhounds on Heroic will now do less damage. - http://wowwiki.wikia.com/wiki/Patch_2.1.0
-- Shattered Hand Reavers will do less damage in Heroic difficulty. - http://wowwiki.wikia.com/wiki/Patch_2.1.0
-- The melee damage dealt by Shattered Hand Reavers, Shattered Hand Legionnaires, Shattered Hand Heathens, and Rabid Warhounds has been reduced. - http://wowwiki.wikia.com/wiki/Patch_2.2.0

-- Shattered Hand Sentry 16507,20593 (2.9/3.915H)(4.664/13D) - 19607/27350
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '4040', `MaxMeleeDmg` = '5712' WHERE `entry` = '20593';
-- Shattered Hand Savage 16523,20591 (2.9/3.915H)(4.664/12D) - 19607/27350
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '20591';
-- Shattered Hand Brawler 16593,20582 (2.9/3.915H)(4.66/12D) - 20259/27350
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '20582';
-- Shadowmoon Acolyte 16594,20576 (2.9/3.915H)(4.676/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '22005', `MaxLevelHealth` = '22005', `MinMeleeDmg` = '2939', `MaxMeleeDmg` = '4265' WHERE `entry` = '20576';
-- Shattered Hand Reaver 16699,20590 (2.9/3.915H)(4.66/13D) - 19607/27350
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '6.0', `MinLevelHealth` = '19607', `MaxLevelHealth` = '20259', `MinMeleeDmg` = '1450', `MaxMeleeDmg` = '2142' WHERE `entry` = '16699';
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '16.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '4040', `MaxMeleeDmg` = '5712' WHERE `entry` = '20590';
-- Shattered Hand Legionnaire 16700,20589 (5/14.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.9', `DamageMultiplier` = '6.0', `MinLevelHealth` = '27245', `MaxLevelHealth` = '27245', `MinMeleeDmg` = '1515', `MaxMeleeDmg` = '2142' WHERE `entry` = '16700';
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `DamageMultiplier` = '17.5', `MinLevelHealth` = '59381', `MaxLevelHealth` = '59381', `MinMeleeDmg` = '4418', `MaxMeleeDmg` = '6248' WHERE `entry` = '20589';
-- Shattered Hand Sharpshooter 16704,20594 (2.9/1.75H)(4.66/12) - 19607/12226
-- https://wow.gamepedia.com/Shattered_Hand_Sharpshooter
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '14.0', `MinLevelHealth` = '20259', `MaxLevelHealth` = '20259', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '20594';
-- Grand Warlock Nethekurse (16807,20568) (18.75/25.3126)(6/5.8D) - 107700/149420
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `DamageMultiplier` = '9.0', `MinLevelHealth` = '171187', `MaxLevelHealth` = '171187', `MinMeleeDmg` = '2178', `MaxMeleeDmg` = '3077' WHERE `entry` = '20568';
-- Warchief Kargath Bladefist (16808,20597) (22.75/28H)(6/17D) - 167895/206640
UPDATE `creature_template` SET `HealthMultiplier` = '31.0', `DamageMultiplier` = '17.0', `MinLevelHealth` = '228780', `MaxLevelHealth` = '228780', `MinMeleeDmg` = '4698', `MaxMeleeDmg` = '6643' WHERE `entry` = '20597';
-- Warbringer O'mrogg (16809,20596) (20.75/28.0125H)(5.6/13D) - 153135/206732
UPDATE `creature_template` SET `HealthMultiplier` = '28.0125', `DamageMultiplier` = '13.0', `MinLevelHealth` = '206732', `MaxLevelHealth` = '206732', `MinMeleeDmg` = '4594', `MaxMeleeDmg` = '6496' WHERE `entry` = '20596';
-- Shattered Hand Executioner (17301,20585) (2.9/3.915H)(4.664/14D) - 19607/27350
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '16.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '4292', `MaxMeleeDmg` = '6069' WHERE `entry` = '20585';
-- Creeping Ooze 17356,20565 (2.9/3.915H)(4.664/9.5D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '9.5', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4284' WHERE `entry` = '20565';
-- Shattered Hand Heathen 17420,20587 (2.9/3.915H)(4.66/10D) - 19607/27350
UPDATE `creature_template` SET `HealthMultiplier` = '2.9', `DamageMultiplier` = '6.0', `MinLevelHealth` = '19607', `MaxLevelHealth` = '19607', `MinMeleeDmg` = '1305', `MaxMeleeDmg` = '1842' WHERE `entry` = '17420';
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '13.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3636', `MaxMeleeDmg` = '5141' WHERE `entry` = '20587';
-- Shattered Hand Archer 17427,20579 (3/1.35H)(4/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `DamageMultiplier` = '12.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972', `MinMeleeDmg` = '3030', `MaxMeleeDmg` = '4284' WHERE `entry` = '20579';
-- Shattered Hand Zealot 17462,20595 (1/1.35H)(1/8D) (6761/9431)
UPDATE `creature_template` SET `HealthMultiplier` = '1.35', `DamageMultiplier` = '8.0', `MinLevelHealth` = '9431', `MaxLevelHealth` = '9431', `MinMeleeDmg` = '2525', `MaxMeleeDmg` = '3570' WHERE `entry` = '20595';
-- Shattered Hand Gladiator 17464,20586 (2.9/3.915H)(4.66/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '4040', `MaxMeleeDmg` = '5712' WHERE `entry` = '20586';
-- Shattered Hand Centurion 17465,20583 (3.9/5.265H)(5/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '6.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '45409', `MaxLevelHealth` = '45409', `MinMeleeDmg` = '4040', `MaxMeleeDmg` = '5712' WHERE `entry` = '20583';
-- Heathen Guard 17621,20569 (1.25/1.6875H)(3.22/4.18D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '5.5', `MinLevelHealth` = '17465', `MaxLevelHealth` = '17465', `MinMeleeDmg` = '972', `MaxMeleeDmg` = '1374' WHERE `entry` = '20569';
-- Sharpshooter Guard 17622,20578 (1.25/1.6875H)(3/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '17465', `MaxLevelHealth` = '17465', `MinMeleeDmg` = '2121', `MaxMeleeDmg` = '2999' WHERE `entry` = '20578';
-- Reaver Guard 17623,20575 (1.25/1.6875H)(3/10D) - 8451/11789
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `DamageMultiplier` = '12.0', `MinLevelHealth` = '17465', `MaxLevelHealth` = '17465', `MinMeleeDmg` = '2121', `MaxMeleeDmg` = '2999' WHERE `entry` = '20575';
-- Rabid Warhound 17669,20574 (1.45/1.75H)(1.8/3D)
UPDATE `creature_template` SET `HealthMultiplier` = '1.45', `DamageMultiplier` = '2.5', `MinLevelHealth` = '9173', `MaxLevelHealth` = '9173', `MinMeleeDmg` = '413', `MaxMeleeDmg` = '581' WHERE `entry` = '17669';
UPDATE `creature_template` SET `HealthMultiplier` = '1.75', `DamageMultiplier` = '12.0', `MinLevelHealth` = '19211', `MaxLevelHealth` = '19211', `MinMeleeDmg` = '1326', `MaxMeleeDmg` = '1874' WHERE `entry` = '20574';
-- Shattered Hand Houndmaster 17670,20588 (2.9/3.91H)(4.66/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '14.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3787', `MaxMeleeDmg` = '5355' WHERE `entry` = '20588';
-- Shattered Hand Champion 17671,20584 (3.9/5.265H)(5/14D)
UPDATE `creature_template` SET `DamageMultiplier` = '7.0' WHERE `entry` = 17671;
UPDATE `creature_template` SET `HealthMultiplier` = '6.5', `DamageMultiplier` = '16.0', `MinLevelHealth` = '45409', `MaxLevelHealth` = '45409', `MinMeleeDmg` = '4292', `MaxMeleeDmg` = '6069' WHERE `entry` = '20584';
-- Shadowmoon Darkcaster 17694,20577 (2.9/3.915H)(4.676/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '15.0', `MinLevelHealth` = '22005', `MaxLevelHealth` = '22005', `MinMeleeDmg` = '3135', `MaxMeleeDmg` = '4549' WHERE `entry` = '20577';
-- Shattered Hand Assassin 17695,20580 (2.9/3.915H)(5/14D)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `DamageMultiplier` = '16.0', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `MinMeleeDmg` = '3434', `MaxMeleeDmg` = '4855' WHERE `entry` = '20580';
-- Shattered Hand Blood Guard 17461,20581 (3.9H)(5/19D) - 28006
-- Blood Guard Porung 20923,20993 (14H)(7.5/7.5D) - 118080
UPDATE `creature_template` SET `HealthMultiplier` = '16.0', `DamageMultiplier` = '21', `MinLevelHealth` = '118080', `MaxLevelHealth` = '118080', `MinMeleeDmg` = '2479', `MaxMeleeDmg` = '3506' WHERE `entry` = '20993';

-- ============================================================================================================================================================================
-- Tempest Keep: The Arcatraz
-- ============================================================================================================================================================================

-- Arcatraz Defender 20857,21585 
-- Arcatraz Warder 20859,21587 
-- Protean Nightmare 20864,21608 (6.5/8.1H)(2/5D) - 46677/58166
UPDATE `creature_template` SET `DamageMultiplier` = '6.0' WHERE `entry` = 20864;
UPDATE `creature_template` SET `DamageMultiplier` = '15.0' WHERE `entry` = 21608;
-- Protean Horror 20865,21607 (1.25/1.25H)(4/9D) - 8178/8178
UPDATE `creature_template` SET `DamageMultiplier` = '3.0' WHERE `entry` = 20865;
UPDATE `creature_template` SET `DamageMultiplier` = '7.0' WHERE `entry` = 21607;
-- Soul Devourer 20866,21614 (6.5/8.1H)(8/20D) - 45409/56587
UPDATE `creature_template` SET `DamageMultiplier` = '12.0' WHERE `entry` = 20866;
UPDATE `creature_template` SET `DamageMultiplier` = '24.0' WHERE `entry` = 21614;
-- Death Watcher 20867,21591 (6.5/8.1H)(8/20D) - 45409/56587
UPDATE `creature_template` SET `HealthMultiplier` = '8', `MinLevelHealth` = '55888', `MaxLevelHealth` = '55888', `DamageMultiplier` = '12.0' WHERE `entry` = 20867;
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '69860', `MaxLevelHealth` = '69860', `DamageMultiplier` = '24.0' WHERE `entry` = 21591;
-- Entropic Eye 20868,21593 (7/8.1H)(7/17.5D) - 39123/45271
UPDATE `creature_template` SET `HealthMultiplier` = '8.6', `MinLevelHealth` = '60080', `MaxLevelHealth` = '60080', `DamageMultiplier` = '11.0' WHERE `entry` = 20868;
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '69860', `MaxLevelHealth` = '69860', `DamageMultiplier` = '24.0' WHERE `entry` = 21593;
-- Arcatraz Sentinel 20869,21586+ (16.5/22.3H)(7/20D) - 115269/155788
UPDATE `creature_template` SET `DamageMultiplier` = '14.0' WHERE `entry` = 21586;
-- Zereketh the Unbound (20870,21626) (20/27H)(6/10D) - 118060/159505 - SPELL_SCHOOL_SHADOW
UPDATE `creature_template` SET `DamageMultiplier` = '8.0' WHERE `entry` = 20870;
UPDATE `creature_template` SET `DamageMultiplier` = '13.0' WHERE `entry` = 21626;
-- Negaton Warp-Master 20873,21605 (6/8.1H)(8.75/22D) - 41916/56587
-- Negaton Screamer 20875,21604 (7/8.1H)(7/17.5D) - 39123/45271
-- Eredar Soul-Eater 20879,21595+ (7/8.1H)(8/30D) - 39123/45271
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `DamageMultiplier` = '12.0' WHERE `entry` = '20879';
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '55890', `MaxLevelHealth` = '55890', `DamageMultiplier` = '34.0' WHERE `entry` = '21595';
-- Eredar Deathbringer 20880,21594+ (7/8.1H)(8/20D) - 39123/45271
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `DamageMultiplier` = '12.0' WHERE `entry` = '20880';
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '55890', `MaxLevelHealth` = '55890', `DamageMultiplier` = '24.0' WHERE `entry` = '21594';
-- Unbound Devastator 20881,21619 (7/8.1H)(8/20D) - 48902/56587
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 20881;
UPDATE `creature_template` SET `DamageMultiplier` = '22.0' WHERE `entry` = 21619;
-- Skulking Witch 20882,21613 (6.5/8.1H)(8/20D) - 45409/56587
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 20882;
UPDATE `creature_template` SET `DamageMultiplier` = '22.0' WHERE `entry` = 21613;
-- Spiteful Temptress 20883,21615 (7/8.1H)(6/25D) - 39123/45271
UPDATE `creature_template` SET `DamageMultiplier` = '8.0' WHERE `entry` = 20883;
UPDATE `creature_template` SET `DamageMultiplier` = '25.0' WHERE `entry` = 21615;
-- Dalliah the Doomsayer (20885,21590) (18/24H)(8.5/8.5D) - 132840/177120
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 20885;
UPDATE `creature_template` SET `DamageMultiplier` = '15.0' WHERE `entry` = 21590;
-- Wrath-Scryer Soccothrates (20886,21624) (18/24H)(8/15D) - 132840/178220
UPDATE `creature_template` SET `DamageMultiplier` = '11.0' WHERE `entry` = 20886;
UPDATE `creature_template` SET `DamageMultiplier` = '24.0' WHERE `entry` = 21624;
-- Ethereum Slayer 20896,21596 (3/4H)(5.35/10D) - 20958/27944
UPDATE `creature_template` SET `DamageMultiplier` = '7.0' WHERE `entry` = 20896;
UPDATE `creature_template` SET `DamageMultiplier` = '14.0' WHERE `entry` = 21596;
-- Ethereum Wave-Caster 20897,21597 (3/4H)(5.35/6D) - 16767/22356
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '6.0', `MinLevelHealth` = '16767', `MaxLevelHealth` = '16767', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '1986' WHERE `entry` = '20897';
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `DamageMultiplier` = '10', `MinLevelHealth` = '22356', `MaxLevelHealth` = '22356', `MinMeleeDmg` = '1992', `MaxMeleeDmg` = '2814' WHERE `entry` = '21597';
-- Gargantuan Abyssal 20898,21598 (10/13.5H)(10/25D) - 71810/96944
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '71810', `MaxLevelHealth` = '71810', `MinMeleeDmg` = '3337', `MaxMeleeDmg` = '4720' WHERE `entry` = '20898';
UPDATE `creature_template` SET `HealthMultiplier` = '15', `DamageMultiplier` = '30.0', `MinLevelHealth` = '107715', `MaxLevelHealth` = '107715', `MinMeleeDmg` = '7188', `MaxMeleeDmg` = '10166' WHERE `entry` = '21598';
-- Unchained Doombringer 20900,21621 (9/12.5H)(10/20D) - 64629/89763
UPDATE `creature_template` SET `DamageMultiplier` = '14.0' WHERE `entry` = 20900;
UPDATE `creature_template` SET `DamageMultiplier` = '24.0' WHERE `entry` = 21621;
-- Sargeron Archer 20901,21610 (6/8.1H)(6.75/17D) - 33534/45271
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 20901;
UPDATE `creature_template` SET `DamageMultiplier` = '21.0' WHERE `entry` = 21610;
-- Sargeron Hellcaller 20902,21611 (6/8.1H)(6.75/17D) - 33534/45271
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 20902;
UPDATE `creature_template` SET `DamageMultiplier` = '21.0' WHERE `entry` = 21611;
-- Warden Mellichar 20904,21622 
-- Blazing Trickster 20905,21589 (12/16H)(6/14D) - 67068/89424
-- Phase-Hunter 20906,21606 (11/15H)(9.015/24D) - 81180/110700
UPDATE `creature_template` SET `DamageMultiplier` = '12.0' WHERE `entry` = 20906;
UPDATE `creature_template` SET `DamageMultiplier` = '24.0' WHERE `entry` = 21606;
-- Akkiris Lightning-Waker 20908,21617 (12/16H)(6.514/14D) - 70836/94448 - SPELL_SCHOOL_NATURE
-- Sulfuron Magma-Thrower 20909,21616 (12/16H)(7.014/17.5D) - 70836/94448
-- Twilight Drakonaar 20910,21618 (12/16H)(8/24D) - 88560/118080
UPDATE `creature_template` SET `DamageMultiplier` = '12.0' WHERE `entry` = 20910;
UPDATE `creature_template` SET `DamageMultiplier` = '24.0' WHERE `entry` = 21618;
-- Blackwing Drakonaar 20911,21588 (12/16H)(8/20D) - 88560/118080
UPDATE `creature_template` SET `DamageMultiplier` = '12.0' WHERE `entry` = 20911;
UPDATE `creature_template` SET `DamageMultiplier` = '24.0' WHERE `entry` = 21588;
-- Harbinger Skyriss (20912,21599) (25/33H)(4/8D) - 147575/194799
UPDATE `creature_template` SET `DamageMultiplier` = '6.0' WHERE `entry` = 20912;
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 21599;
-- Millhouse Manastorm 20977,21602 
-- Unbound Void Zone 21101,21620 
-- Arcane Warder Target 21186
-- Defender Corpse 21303,21592 
-- Warder Corpse 21304,21623 
-- Sightless Eye 21346,21612 
-- Protean Spawn 21395,21609 (0.5/0.6885H)(3.3/4.2D) - 3271/4655
-- Negaton Field 21414,21603 
-- Harbinger Skyriss 21466,21600 (3/4H)(4/8D) - 17709/120020
UPDATE `creature_template` SET `DamageMultiplier` = '6.0' WHERE `entry` = 21466;
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 21600;
-- Harbinger Skyriss 21467,21601 (6/8H)(4/8D) - 17709/120020
UPDATE `creature_template` SET `DamageMultiplier` = '6.0' WHERE `entry` = 21467;
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 21601;
-- Ethereum Life-Binder 21702,22346 (3/4H)(5.35/12D) - 16767/22356
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '6.0', `MinLevelHealth` = '16767', `MaxLevelHealth` = '16767', `MinMeleeDmg` = '1406', `MaxMeleeDmg` = '1986' WHERE `entry` = '21702';
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `DamageMultiplier` = '15.0', `MinLevelHealth` = '22356', `MaxLevelHealth` = '22356', `MinMeleeDmg` = '3515', `MaxMeleeDmg` = '4966' WHERE `entry` = '22346';
-- Udalo 21962
-- Third Fragment Guardian 22892 (3H)(3D) - 20958

-- ============================================================================================================================================================================
-- Tempest Keep: The Botanica
-- ============================================================================================================================================================================

-- High Botanist Freywinn (17975,21558) (15.75/21.2625H)(6/13D)- 92972/125513
UPDATE `creature_template` SET `DamageMultiplier` = '8' WHERE `entry` = 17975;
UPDATE `creature_template` SET `DamageMultiplier` = '15' WHERE `entry` = 21558;
-- Commander Sarannis (17976,21551) (15/20.25H)(8.3/13D) - 110700/149445
UPDATE `creature_template` SET `DamageMultiplier` = '8.3' WHERE `entry` = 17976;
UPDATE `creature_template` SET `DamageMultiplier` = '15' WHERE `entry` = 21551;
-- Warp Splinter (17977,21582) (18/24.3H)(6/15D) - 132840/179334
UPDATE `creature_template` SET `DamageMultiplier` = '8' WHERE `entry` = 17977;
UPDATE `creature_template` SET `DamageMultiplier` = '17' WHERE `entry` = 21582;
-- Thorngrin the Tender 17978,21581 (8.3/13D)
UPDATE `creature_template` SET `HealthMultiplier` = '12.5', `DamageMultiplier` = '9.0', `MinLevelHealth` = '73788', `MaxLevelHealth` = '73788', `MinMeleeDmg` = '2178', `MaxMeleeDmg` = '3077' WHERE `entry` = '17978';
UPDATE `creature_template` SET `HealthMultiplier` = '16.875', `DamageMultiplier` = '17.0', `MinLevelHealth` = '99613', `MaxLevelHealth` = '99613', `MinMeleeDmg` = '4114', `MaxMeleeDmg` = '5812' WHERE `entry` = '21581';
-- Laj (17980,21559) (16.5/22.275H)(7.5/13D) - 121770/164389
UPDATE `creature_template` SET `DamageMultiplier` = '8' WHERE `entry` = 17980;
UPDATE `creature_template` SET `DamageMultiplier` = '15' WHERE `entry` = 21559;
-- Bloodwarder Protector 17993,21548 
-- Bloodwarder Falconer 17994,21545 
-- Bloodfalcon 18155,21544 
-- Bloodwarder Steward 18404,21549 (3/4.05H)(4.66/12.5D) - 20283/28293
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '33805', `MaxLevelHealth` = '33805', `DamageMultiplier` = '5' WHERE `entry` = 18404;
UPDATE `creature_template` SET `HealthMultiplier` = '6', `MinLevelHealth` = '40566', `MaxLevelHealth` = '40566', `DamageMultiplier` = '15.0' WHERE `entry` = 21549;
-- Tempest Forge Peacekeeper 18405,21578 (4/12H)(4.66/14.5D)- 22356/70836 - SPELL_SCHOOL_ARCANE Prenerf
UPDATE `creature_template` SET `HealthMultiplier` = '10', `MinLevelHealth` = '41321', `MaxLevelHealth` = '41321', `DamageMultiplier` = '7' WHERE `entry` = 18405;
UPDATE `creature_template` SET `HealthMultiplier` = '14', `MinLevelHealth` = '82642', `MaxLevelHealth` = '82642', `DamageMultiplier` = '14.5' WHERE `entry` = 21578;
-- Bloodwarder Greenkeeper 18419,21546 
-- Sunseeker Geomancer 18420,21574 
-- Sunseeker Researcher 18421,21577 
-- Sunseeker Botanist 18422,21570 
-- Frayer 18587,21552 (1/5D)
UPDATE `creature_template` SET `DamageMultiplier` = '2' WHERE `entry` = 18587;
UPDATE `creature_template` SET `DamageMultiplier` = '6' WHERE `entry` = 21552;
-- Sunseeker Chemist 19486,21572 
-- Sunseeker Channeler 19505,21571 
-- Sunseeker Gene-Splicer 19507,21573 
-- Sunseeker Herbalist 19508,21576 
-- Sunseeker Harvester 19509,21575 
-- Nethervine Inciter 19511,21563 (7/14D)
UPDATE `creature_template` SET `DamageMultiplier` = '8' WHERE `entry` = 19511;
UPDATE `creature_template` SET `DamageMultiplier` = '17' WHERE `entry` = 21563;
-- Nethervine Reaper 19512,21564 (4.66/12D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '7', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958' WHERE `entry` = '19512';
UPDATE `creature_template` SET `HealthMultiplier` = '4.05', `DamageMultiplier` = '15.0', `MinLevelHealth` = '28293', `MaxLevelHealth` = '29083' WHERE `entry` = '21564';
-- Mutate Fear-Shrieker 19513,21560 (2/10D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '4.0', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `MinMeleeDmg` = '631', `MaxMeleeDmg` = '893' WHERE `entry` = '19513';
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '12.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '35905', `MinMeleeDmg` = '3282', `MaxMeleeDmg` = '4720' WHERE `entry` = '21560';
-- Greater Frayer 19557,21555 (2/8D)
UPDATE `creature_template` SET `DamageMultiplier` = '4' WHERE `entry` = 19557;
UPDATE `creature_template` SET `DamageMultiplier` = '10' WHERE `entry` = 21555;
-- Mutate Fleshlasher 19598,21561 (0.7/0.945H)(4.66/10D) - 4890/6602
UPDATE `creature_template` SET `HealthMultiplier` = '0.7', `DamageMultiplier` = '5.5', `MinLevelHealth` = '4890', `MaxLevelHealth` = '4890', `MinMeleeDmg` = '1389', `MaxMeleeDmg` = '1964' WHERE `entry` = '19598';
UPDATE `creature_template` SET `HealthMultiplier` = '0.945', `DamageMultiplier` = '14.0', `MinLevelHealth` = '6602', `MaxLevelHealth` = '6786', `MinMeleeDmg` = '3535', `MaxMeleeDmg` = '5083' WHERE `entry` = '21561';
-- Frayer Wildling 19608,21554 (1/4D)
UPDATE `creature_template` SET `DamageMultiplier` = '3' WHERE `entry` = 19608;
UPDATE `creature_template` SET `DamageMultiplier` = '6' WHERE `entry` = 21554;
-- Bloodwarder Mender 19633,21547 
-- Nethervine Trickster 19843,21565 (4.66/12D)
UPDATE `creature_template` SET `DamageMultiplier` = '7' WHERE `entry` = 19843;
UPDATE `creature_template` SET `DamageMultiplier` = '15' WHERE `entry` = 21565;
-- Mutate Horror 19865,21562 (2/11D)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `DamageMultiplier` = '4.0', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958' WHERE `entry` = '19865';
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `DamageMultiplier` = '13.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '35905' WHERE `entry` = '21562';
-- Thorn Lasher 19919,21580 (2/12.5D)
-- Thorn Flayer 19920,21579 (2/10D)
-- Sapling 19949,21567 (2/7D)
UPDATE `creature_template` SET `DamageMultiplier` = '3' WHERE `entry` = 19949;
UPDATE `creature_template` SET `DamageMultiplier` = '8' WHERE `entry` = 21567;
-- Frayer Protector 19953,21553 
-- White Seedling 19958,21583 
-- Blue Seedling 19962,21550 
-- Red Seedling 19964,21566 
-- Green Seedling 19969,21557 
-- Summoned Bloodwarder Reservist 20078,21569 
-- Summoned Bloodwarder Mender 20083,21568 
-- Mutate Fleshlasher 25354 (4.7D)

-- ============================================================================================================================================================================
-- Tempest Keep: The Mechanar
-- ============================================================================================================================================================================

-- Sunseeker Netherbinder 20059,21541 (9/20D)
-- Tempest-Forge Patroller 19166,21543+ (5/6.75H)(2/12D) - 35905/48472
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '35905', `MaxLevelHealth` = '35905', `DamageMultiplier` = '5' WHERE `entry` = '19166';
UPDATE `creature_template` SET `HealthMultiplier` = '6.75', `MinLevelHealth` = '48472', `MaxLevelHealth` = '48472', `DamageMultiplier` = '18' WHERE `entry` = '21543';
-- Gatewatcher Gyro-Kill (19218,21525) (12.5/16.875H)(8/8D) - 92250/124538
UPDATE `creature_template` SET `HealthMultiplier` = '12.5', `MinLevelHealth` = '92250', `MaxLevelHealth` = '92250', `DamageMultiplier` = '8' WHERE `entry` = '19218';
UPDATE `creature_template` SET `HealthMultiplier` = '16.875', `MinLevelHealth` = '124538', `MaxLevelHealth` = '124538', `DamageMultiplier` = '14' WHERE `entry` = '21525';
-- Gatewatcher Iron-Hand (19710,21526) (16.5/22.275H)(8/8D) - 121770/164389
UPDATE `creature_template` SET `HealthMultiplier` = '16.5', `MinLevelHealth` = '121770', `MaxLevelHealth` = '121770', `DamageMultiplier` = '8' WHERE `entry` = '19710';
UPDATE `creature_template` SET `HealthMultiplier` = '22.275', `MinLevelHealth` = '164390', `MaxLevelHealth` = '164390', `DamageMultiplier` = '18' WHERE `entry` = '21526';
-- Tempest-Forge Destroyer 19735,21542+ (12/16.2H)(8/24D) - 86172/116332
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '8' WHERE `entry` = '19735';
UPDATE `creature_template` SET `HealthMultiplier` = '16.2', `MinLevelHealth` = '116332', `MaxLevelHealth` = '116332', `DamageMultiplier` = '26' WHERE `entry` = '21542';
-- Mechano-Lord Capacitus (19219,21533) (16.5/35H)(8/8D) - 121770/258300
UPDATE `creature_template` SET `HealthMultiplier` = '16.5', `MinLevelHealth` = '121770', `MaxLevelHealth` = '121770', `DamageMultiplier` = '8' WHERE `entry` = '19219';
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '258300', `MaxLevelHealth` = '258300', `DamageMultiplier` = '14' WHERE `entry` = '21533';
-- Nethermancer Sepethrea (19221,21536) (16.5/22.275H)(6.5/6.5D) - 97400/131489
UPDATE `creature_template` SET `HealthMultiplier` = '16.5', `MinLevelHealth` = '97400', `MaxLevelHealth` = '97400', `DamageMultiplier` = '6.5' WHERE `entry` = '19221';
UPDATE `creature_template` SET `HealthMultiplier` = '22.275', `MinLevelHealth` = '131489', `MaxLevelHealth` = '131489', `DamageMultiplier` = '7.5' WHERE `entry` = '21536';
-- Pathaleon the Calculator (19220,21537) (17/22.95H)(6/16D) - 100351/135474
UPDATE `creature_template` SET `HealthMultiplier` = '17', `MinLevelHealth` = '100351', `MaxLevelHealth` = '100351', `DamageMultiplier` = '6' WHERE `entry` = '19220';
UPDATE `creature_template` SET `HealthMultiplier` = '22.95', `MinLevelHealth` = '135474', `MaxLevelHealth` = '135474', `DamageMultiplier` = '18' WHERE `entry` = '21537';
-- Nether Wraith (21062,21535) (1.5/2.025H)(2.143/8D) - 8384/11318

-- ============================
-- Trash
-- ============================

-- Arcane Servant 20478,21521
UPDATE `creature_template` SET `HealthMultiplier` = '0.75', `DamageMultiplier` = '3.0', `MinLevelHealth` = '4192', `MaxLevelHealth` = '4192', `MinMeleeDmg` = '492', `MaxMeleeDmg` = '695' WHERE `entry` = '20478';
UPDATE `creature_template` SET `HealthMultiplier` = '1.75', `DamageMultiplier` = '4.0', `MinLevelHealth` = '9780', `MaxLevelHealth` = '9780', `MinMeleeDmg` = '656', `MaxMeleeDmg` = '927' WHERE `entry` = '21521';

-- ============================================================================================================================================================================
-- Magisters' Terrace
-- ============================================================================================================================================================================

-- Selin Fireheart (24723,25562) (27.5/37.5H)(3.5/13D) - 164000/215000
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 24723;
UPDATE `creature_template` SET `DamageMultiplier` = '20.0' WHERE `entry` = 25562;
-- Fel Crystal (24722,25552) (0.9/1.8H) - 6287/12575 - Pre 2.4.3
UPDATE `creature_template` SET `HealthMultiplier` = '1.0', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986' WHERE `entry` = '24722';
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972' WHERE `entry` = '25552';
-- Vexallus (24744,25573) (25/34H)(8/14.5D) - 143600/195296 - SPELL_SCHOOL_ARCANE
UPDATE `creature_template` SET `DamageMultiplier` = '8.0' WHERE `entry` = 24744;
UPDATE `creature_template` SET `DamageMultiplier` = '14.5' WHERE `entry` = 25573;
-- Pure Energy 24745 
-- Priestess Delrissa (Priest)(24560,25560) (4.4/6H)(2.75/13D) - 24591.6/33534
-- Apoko (Shaman)(24553,25541) (3.85/5.25H)(4/12D) - 21518/29342
-- Eramas Brightblaze (Monk)(24554,25550) (2.89/4H)(10/17D) - 20190/27944
-- Garaxxas (Hunter)(24555,25555) (3.85/5.25H)(6/13.5D) - 21518/29342
-- Sliver (Ravager)(24552,25564) (1.65/2.25H)(5/12D) - 11527/15719
-- Zelfan (Engineer)(24556,25579) (2.89/4H)(3/10D) - 16152/22356
-- High Explosive Sheep 24715
-- Kagani Nightstrike (Rogue)(24557,25556) (2.89/4H)(5/16D) - 20190/27944 - Pre 2.4.3
UPDATE `creature_template` SET `DamageMultiplier` = '6.0', `MinMeleeDmg` = '1136', `MaxMeleeDmg` = '1607' WHERE `entry` = '24557';
UPDATE `creature_template` SET `DamageMultiplier` = '18.0', `MinMeleeDmg` = '3409', `MaxMeleeDmg` = '4820' WHERE `entry` = '25556';
-- Ellrys Duskhallow (Warlock)(24558,25549) (2.89/4H)(4/12D) - 16152/22356
-- Fizzle (Imp)(24656,25553) (0.4/0.36H)(1.75/3D) - 1956/1760
-- Warlord Salaris (Warrior)(24559,25574) (3.85/5.25H)(4.2/8D) - 26896/36677 - Pre 2.4.3
UPDATE `creature_template` SET `DamageMultiplier` = '5.0', `MinMeleeDmg` = '2272', `MaxMeleeDmg` = '3213' WHERE `entry` = '24559';
UPDATE `creature_template` SET `DamageMultiplier` = '10.0', `MinMeleeDmg` = '4545', `MaxMeleeDmg` = '6426' WHERE `entry` = '25574';
-- Yazzai (Mage)(24561,25578) (3.85/5.25H)(4/12D) - 21518/29342
-- Kael'thas Sunstrider (24664,24857) (30/40H)(4.75/13D) - 177090/236120
UPDATE `creature_template` SET `DamageMultiplier` = '10.0' WHERE `entry` = 24664;
UPDATE `creature_template` SET `DamageMultiplier` = '20.0' WHERE `entry` = 24857;
-- Phoenix (24674) (5H/1D) - 34930
-- Phoenix Egg (24675) (1H/1D) - 6986
-- Arcane Sphere 24708 

-- ============================
-- Trash
-- ============================

-- Sunblade Mage Guard 24683,25568 (3.62/4.82H)(4/13D)
UPDATE `creature_template` SET `DamageMultiplier` = '5.0' WHERE `entry` = 24683;
UPDATE `creature_template` SET `DamageMultiplier` = '15.0' WHERE `entry` = 25568;
-- Sunblade Blood Knight 24684,25565 (3.62/4.82H)(8/13.5D)
UPDATE `creature_template` SET `DamageMultiplier` = '9.0' WHERE `entry` = 24684;
UPDATE `creature_template` SET `DamageMultiplier` = '16.0' WHERE `entry` = 25565;
-- Sunblade Magister 24685,25569 (2.9/4H)(4/12D)
UPDATE `creature_template` SET `DamageMultiplier` = '4.0' WHERE `entry` = 24685;
UPDATE `creature_template` SET `DamageMultiplier` = '14.0' WHERE `entry` = 25569;
-- Sunblade Warlock 24686,25572 (3/4H)(3.5/12.5D) - Pre 2.4.3
UPDATE `creature_template` SET `DamageMultiplier` = '4.0', `MinMeleeDmg` = '937', `MaxMeleeDmg` = '1324' WHERE `entry` = 24686;
UPDATE `creature_template` SET `DamageMultiplier` = '14.0', `MinMeleeDmg` = '3281', `MaxMeleeDmg` = '4635' WHERE `entry` = 25572;
-- Sunblade Physician 24687,25570 (3/4H)(5.5/13D)
UPDATE `creature_template` SET `DamageMultiplier` = '6.5' WHERE `entry` = 24687;
UPDATE `creature_template` SET `DamageMultiplier` = '15.0' WHERE `entry` = 25570;
-- Wretched Skuller 24688,25577 (2.5/3.5H)(4/11D) :5:10
UPDATE `creature_template` SET `DamageMultiplier` = '0.8' WHERE `entry` = 24688;
UPDATE `creature_template` SET `DamageMultiplier` = '1.1' WHERE `entry` = 25577;
-- Wretched Bruiser 24689,25575 (2.5/3.5H)(4.5/12D) :5:10
UPDATE `creature_template` SET `DamageMultiplier` = '0.9' WHERE `entry` = 24689;
UPDATE `creature_template` SET `DamageMultiplier` = '1.2' WHERE `entry` = 25575;
-- Wretched Husk 24690,25576 (2.5/3.5H)(4.5/12D) :5:10
UPDATE `creature_template` SET `DamageMultiplier` = '0.9' WHERE `entry` = 24690;
UPDATE `creature_template` SET `DamageMultiplier` = '1.2' WHERE `entry` = 25576;
-- Coilskar Witch 24696,25547 (3/4H)(4/12D)
UPDATE `creature_template` SET `DamageMultiplier` = '5.0' WHERE `entry` = 24696;
UPDATE `creature_template` SET `DamageMultiplier` = '14.0' WHERE `entry` = 25547;
-- Sister of Torment 24697,25563 (3/4H)(3.5/13D)
UPDATE `creature_template` SET `DamageMultiplier` = '4.5' WHERE `entry` = 24697;
UPDATE `creature_template` SET `DamageMultiplier` = '15.0' WHERE `entry` = 25563;
-- Ethereum Smuggler 24698,25551 (3/4H)(5.5/12.25D)
UPDATE `creature_template` SET `DamageMultiplier` = '6.5' WHERE `entry` = 24698;
UPDATE `creature_template` SET `DamageMultiplier` = '15.0' WHERE `entry` = 25551;
-- Brightscale Wyrm 24761,25545 (0.8/1.05H)(2/5D)
-- Sunblade Keeper 24762,25567 (2.9/4H)(8.5/16D)
UPDATE `creature_template` SET `DamageMultiplier` = '8.5' WHERE `entry` = 24762;
UPDATE `creature_template` SET `DamageMultiplier` = '16.0' WHERE `entry` = 25567;
-- Sunblade Sentinel 24777,25571 (9.2/12H)(8/18D) - 66065/86172
UPDATE `creature_template` SET `DamageMultiplier` = '16.0' WHERE `entry` = 24777;
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '107715', `MaxLevelHealth` = '107715', `DamageMultiplier` = '36.0' WHERE `entry` = 25571;
-- Sunblade Imp 24815,25566 (0.65/0.9H)(1/6D)

