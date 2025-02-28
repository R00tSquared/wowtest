-- Just apply the part of the file
-- that applies to your current Content Release State
-- Comment the rest out

-- Dungeons		-- Dungeons should require groups to play around the boss mechanics, tune so they cant just out-dps them
-- T4 Content	-- Pre-nerf Value +	60% (2.0) 30% (2.1) 0% (2.4)
-- T5 Content	-- Pre-nerf Value +	50% (2.0) 40% (2.1) 20% (2.4)
-- T6 Content	-- Pre-nerf Value +	80% (2.0) 40% (2.1) 30% (2.4)
-- Zul'Aman		-- Pre-nerf Value + 40% (2.3) 30% (2.4)
-- SWP			-- Pre-nerf Value + Server depended
-- &dmg			-- damage increased according to this value

-- 																================================
-- 																=====    Dungeon Tuning    =====
-- 																================================
-- 
-- Dungeon Tuning is delicate this is just a prototype for if we want to tune certain bosses, only MGT Bosses are tuned for now

-- Dungeon Difficulty Ranking
-- Easy:	Hellfire Ramparts	The Slave Pens	The Mechanar		Mana-Tombs
-- Medium:	The Blood Furnace	The Underbog	Auchenai Crypts		Sethekk Halls		Old Hillsbrad Foothills	The Botanica
-- Hard:	The Shattered Halls	The Steamvault	Shadow Labyrinth	The Black Morass	The Arcatraz
-- Tryhard:	Magisters' Terrace

--
-- =====================================================================================================
-- Hellfire Citadel: Hellfire Ramparts
-- =====================================================================================================

-- Watchkeeper Gargolmar (17306,18436) (6.25/14H)(6/20D) - 33381/103320
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '18436';
-- Omor the Unscarred (17308,18433) (14/14H)(7/8D) - 59836/82642
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '147575', `MaxLevelHealth` = '147575' WHERE `entry` = '18433';
-- Nazan (17536,18432) (6.25/12H)(4.376/12D) - 33381/88560
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '132840', `MaxLevelHealth` = '132840' WHERE `entry` = '18432';
-- Vazruden (17537,18434) (3.6/8.5H)(5.3/8D) - 19228/62730
UPDATE `creature_template` SET `HealthMultiplier` = '13', `MinLevelHealth` = '95940', `MaxLevelHealth` = '95940' WHERE `entry` = '18434';

-- =====================================================================================================
-- Hellfire Citadel: The Blood Furnace
-- =====================================================================================================

-- The Maker (17381,18621) (7.25/14H)(6/10D) - 38722/100534
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '143620', `MaxLevelHealth` = '143620' WHERE `entry` = '18621';
-- Broggok (17380,18601) (8/16H)(4.7/15D) - 30960/82656
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980' WHERE `entry` = '18601';
-- Keli'dan the Breaker (17377,18607) (9/20H)(6/14D) - 34830/103302
UPDATE `creature_template` SET `HealthMultiplier` = '33', `MinLevelHealth` = '170478', `MaxLevelHealth` = '170478' WHERE `entry` = '18607';

-- =====================================================================================================
-- Hellfire Citadel: The Shattered Halls
-- =====================================================================================================

-- Grand Warlock Nethekurse (16807,20568) (18.75/25.3126)(6/5.8D) - 107700/149420
UPDATE `creature_template` SET `HealthMultiplier` = '33', `MinLevelHealth` = '194799', `MaxLevelHealth` = '194799' WHERE `entry` = '20568';
-- Shattered Hand Blood Guard / Blood Guard Porung (17461,20993) (3.9/14H)(5/7.5D) - 28006/118080
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '20993';
-- Warbringer O'mrogg (16809,20596) (20.75/28.0125H)(5.6/13D) - 153135/206732
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '258300', `MaxLevelHealth` = '258300' WHERE `entry` = '20596';
-- Warchief Kargath Bladefist (16808,20597) (22.75/28H)(6/17D) - 167895/206640
UPDATE `creature_template` SET `HealthMultiplier` = '38', `MinLevelHealth` = '280440', `MaxLevelHealth` = '280440' WHERE `entry` = '20597';

-- =====================================================================================================
-- Coilfang Reservoir: The Slave Pens
-- =====================================================================================================

-- Mennu the Betrayer (17941,19893) (17/22.95H)(5.8/13D) - 77724/132040
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090' WHERE `entry` = '19893';
-- Rokmar the Crackler (17991,19895) (17/22.95H)(6.5/14D) - 97155/169371
UPDATE `creature_template` SET `HealthMultiplier` = '31', `MinLevelHealth` = '228780', `MaxLevelHealth` = '228780' WHERE `entry` = '19895';
-- Quagmirran (17942,19894) (18/24.3H)(8/16D) - 102870/179334
UPDATE `creature_template` SET `HealthMultiplier` = '34', `MinLevelHealth` = '250920', `MaxLevelHealth` = '250920' WHERE `entry` = '19894';
-- Frozen Core (25865,26339) (40/52H) - 303520/394576
UPDATE `creature_template` SET `HealthMultiplier` = '52', `MinLevelHealth` = '394576', `MaxLevelHealth` = '394576' WHERE `entry` = '26339';
-- Ahune (25740,26338) (23/30.25H)(8/9D) - 150000/230000
UPDATE `creature_template` SET `HealthMultiplier` = '38', `MinLevelHealth` = '288344', `MaxLevelHealth` = '288344' WHERE `entry` = '26338';

-- =====================================================================================================
-- Coilfang Reservoir: The Underbog
-- =====================================================================================================

-- Hungarfen (17770,20169) (11/14.85H)(10/20D) - 65054/110054
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '20169';
-- Ghaz'an (18105,20168) (10/13.5H)(6/20D) - 59140/99630
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '20168';
-- Overseer Tidewrath (18107) (12H/4.4D) - 250000
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '118280', `MaxLevelHealth` = '118280' WHERE `entry` = '18107';
-- Swamplord Musel'ek (17826,20183) (11.25/15.1875H)(9/20D) - 53224/89224
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '147575', `MaxLevelHealth` = '147575' WHERE `entry` = '20183';
-- Claw (17827,20165) (9/12.15H)(1/2.5D) - 53226/89667
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '132840', `MaxLevelHealth` = '132840' WHERE `entry` = '20165';
-- The Black Stalker (17882,20184) (16/21.6H)(9.6/10D) - 75696/127505
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '236120', `MaxLevelHealth` = '236120' WHERE `entry` = '20184';

-- =====================================================================================================
-- Coilfang Reservoir: The Steamvault
-- =====================================================================================================

-- Hydromancer Thespia (17797,20629) (17/24H)(6.5/9D) - 100531/141672
UPDATE `creature_template` SET `HealthMultiplier` = '34', `MinLevelHealth` = '200702', `MaxLevelHealth` = '200702' WHERE `entry` = '20629';
-- Mekgineer Steamrigger (17796,20630) (14/17H)(8/10D) - 103320/125460
UPDATE `creature_template` SET `HealthMultiplier` = '24', `MinLevelHealth` = '177120', `MaxLevelHealth` = '177120' WHERE `entry` = '20630';
-- Warlord Kalithresh (17798,20633) (20/32H)(6.5/12D) - 147600/236160
UPDATE `creature_template` SET `HealthMultiplier` = '42', `MinLevelHealth` = '309960', `MaxLevelHealth` = '309960' WHERE `entry` = '20633';

-- =====================================================================================================
-- Auchindoun: Mana-Tombs
-- =====================================================================================================

-- Pandemonius (18341,20267) (12/16.2H)(5.8/20D) - 73392/119556
UPDATE `creature_template` SET `HealthMultiplier` = '21', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980' WHERE `entry` = '20267';
-- Tavarok (18343,20268) (15/20.25H)(5.8/24D) - 91740/149445
UPDATE `creature_template` SET `HealthMultiplier` = '28', `MinLevelHealth` = '206640', `MaxLevelHealth` = '206640' WHERE `entry` = '20268';
-- Nexus-Prince Shaffar (18344,20266) (18/24.3H)(6/4.7D) - 88056/143443
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '206605', `MaxLevelHealth` = '206605' WHERE `entry` = '20266';
-- Yor Void Hound of Shaffar (22930) (35H/15D) - 244343
UPDATE `creature_template` SET `HealthMultiplier` = '43', `MinLevelHealth` = '300398', `MaxLevelHealth` = '300398' WHERE `entry` = '22930';

-- =====================================================================================================
-- Auchindoun: Auchenai Crypts
-- =====================================================================================================

-- Shirrak the Dead Watcher (18371,20318) (15.75/21.2625H)(6/10D) - 77409/122132
UPDATE `creature_template` SET `HealthMultiplier` = '21.2625', `MinLevelHealth` = '122132', `MaxLevelHealth` = '122132' WHERE `entry` = '20318';
-- Exarch Maladaar (18373,20306) (16.5/22.275H)(14/12D) - 97400/131489
UPDATE `creature_template` SET `HealthMultiplier` = '34', `MinLevelHealth` = '200702', `MaxLevelHealth` = '200702' WHERE `entry` = '20306';
-- Avatar of the Martyred (18478,20303) (4.75/6.4125H)(4.994/12.5D) - 30049/47324
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '88560', `MaxLevelHealth` = '88560' WHERE `entry` = '20303';

-- =====================================================================================================
-- Auchindoun: Sethekk Halls
-- =====================================================================================================

-- Darkweaver Syth (18472,20690) (18/22H)(6.3/11D) - 85194/113652
UPDATE `creature_template` SET `HealthMultiplier` = '31', `MinLevelHealth` = '160146', `MaxLevelHealth` = '160146' WHERE `entry` = '20690';
-- Anzu (23035) (40H/20D) - 236120
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150' WHERE `entry` = '23035';
-- Talon King Ikiss (18473,20706) (17/21.9375H)(9.2/15D) - 80461/113329
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '206640', `MaxLevelHealth` = '206640' WHERE `entry` = '20706';

-- =====================================================================================================
-- Auchindoun: Shadow Labyrinth
-- =====================================================================================================

-- Ambassador Hellmaw (18731,20636) (18.25/25H)(6/7D) - 134685/184500
UPDATE `creature_template` SET `HealthMultiplier` = '31', `MinLevelHealth` = '228780', `MaxLevelHealth` = '228780' WHERE `entry` = '20636';
-- Blackheart the Inciter (18667,20637) (18.75/25H)(5.3/10D) - 110750/184500
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '184500', `MaxLevelHealth` = '184500' WHERE `entry` = '20637';
-- Grandmaster Vorpil (18732,20653) (18.75/23.5H)(6/6D) - 110681/138720
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090' WHERE `entry` = '20653';
-- Murmur (18708,20657) (57.5/77.625H) * 0.4 (5/9D) - 392700/572872
UPDATE `creature_template` SET `HealthMultiplier` = '103.2', `MinLevelHealth` = '761616', `MaxLevelHealth` = '761616' WHERE `entry` = '20657';

-- =====================================================================================================
-- Caverns of Time: Old Hillsbrad Foothills
-- =====================================================================================================

-- Lieutenant Drake (17848,20535) (12/16.2H)(4.66/9D) - 78504/119556
UPDATE `creature_template` SET `HealthMultiplier` = '21', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980' WHERE `entry` = '20535';
-- Captain Skarloc (17862,20521) (12/16.2H)(1.5/13D) - 60108/95629
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '147575', `MaxLevelHealth` = '147575' WHERE `entry` = '20521';
-- Epoch Hunter (18096,20531) (15/20.25H)(6.7/20D) - 98130/149445
UPDATE `creature_template` SET `HealthMultiplier` = '32', `MinLevelHealth` = '236160', `MaxLevelHealth` = '236160' WHERE `entry` = '20531';

-- =====================================================================================================
-- Caverns of Time: The Black Morass
-- =====================================================================================================

-- Chrono Lord Deja (17879,20738) (15/20H)(8.6/12D) - 88545/120000
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '151750', `MaxLevelHealth` = '151750' WHERE `entry` = '20738';
-- Infinite Chrono-Lord (21697,21712) (15/20H)(6.013/15D) - 88545/118060
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '147575', `MaxLevelHealth` = '147575' WHERE `entry` = '21712';
-- Temporus (17880,20745) (15/20H)(10/12D) - 110700/151760
UPDATE `creature_template` SET `HealthMultiplier` = '26', `MinLevelHealth` = '197288', `MaxLevelHealth` = '197288' WHERE `entry` = '20745';
-- Aeonus (17881,20737) (20/27H)(13/15D) - 147600/199260
UPDATE `creature_template` SET `HealthMultiplier` = '34', `MinLevelHealth` = '250920', `MaxLevelHealth` = '250920' WHERE `entry` = '20737';

-- =====================================================================================================
-- Tempest Keep: The Mechanar
-- =====================================================================================================

-- Gatewatcher Gyro-Kill (19218,21525) (12.5/16.875H)(8/8D) - 92250/124538
UPDATE `creature_template` SET `HealthMultiplier` = '21', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980' WHERE `entry` = '21525';
-- Gatewatcher Iron-Hand (19710,21526) (16.5/22.275H)(8/8D) - 121770/164389
UPDATE `creature_template` SET `HealthMultiplier` = '27', `MinLevelHealth` = '199260', `MaxLevelHealth` = '199260' WHERE `entry` = '21526';
-- Mechano-Lord Capacitus (19219,21533) (16.5/35H)(8/8D) - 121770/258300
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '295200', `MaxLevelHealth` = '295200' WHERE `entry` = '21533';
-- Nethermancer Sepethrea (19221,21536) (16.5/22.275H)(6.5/6.5D) - 97400/131489
UPDATE `creature_template` SET `HealthMultiplier` = '32', `MinLevelHealth` = '188896', `MaxLevelHealth` = '188896' WHERE `entry` = '21536';
-- Pathaleon the Calculator (19220,21537) (17/22.95H)(6/16D) - 100351/135474
UPDATE `creature_template` SET `HealthMultiplier` = '32', `MinLevelHealth` = '188896', `MaxLevelHealth` = '188896' WHERE `entry` = '21537';

-- =====================================================================================================
-- Tempest Keep: The Botanica
-- =====================================================================================================

-- Commander Sarannis (17976,21551) (15/20.25H)(8.3/13D) - 110700/149445
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700' WHERE `entry` = '17976';
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '184500', `MaxLevelHealth` = '184500' WHERE `entry` = '21551';
-- High Botanist Freywinn (17975,21558) (15.75/21.2625H)(6/13D)- 92972/125513
UPDATE `creature_template` SET `HealthMultiplier` = '15.75', `MinLevelHealth` = '92972', `MaxLevelHealth` = '92972' WHERE `entry` = '17975';
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090' WHERE `entry` = '21558';
-- Thorngrin the Tender (17978,21581) (12.5/16.875H)(8.3/13D) - 73788/99613
UPDATE `creature_template` SET `HealthMultiplier` = '17', `MinLevelHealth` = '100351', `MaxLevelHealth` = '100351' WHERE `entry` = '17978';
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '177090', `MaxLevelHealth` = '177090' WHERE `entry` = '21581';
-- Laj (17980,21559) (16.5/22.275H)(7.5/13D) - 121770/164389
UPDATE `creature_template` SET `HealthMultiplier` = '16.5', `MinLevelHealth` = '121770', `MaxLevelHealth` = '121770' WHERE `entry` = '17980';
UPDATE `creature_template` SET `HealthMultiplier` = '27', `MinLevelHealth` = '199260', `MaxLevelHealth` = '199260' WHERE `entry` = '21559';
-- Warp Splinter (17977,21582) (18/24.3H)(6/15D) - 132840/179334
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '132840', `MaxLevelHealth` = '132840' WHERE `entry` = '17977';
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '221400', `MaxLevelHealth` = '221400' WHERE `entry` = '21582';

-- =====================================================================================================
-- Tempest Keep: The Arcatraz
-- =====================================================================================================

-- Zereketh the Unbound (20870,21626) (20/27H)(6/10D) - 118060/159505
UPDATE `creature_template` SET `HealthMultiplier` = '24', `MinLevelHealth` = '141768', `MaxLevelHealth` = '141768' WHERE `entry` = '20870';
UPDATE `creature_template` SET `HealthMultiplier` = '34', `MinLevelHealth` = '200702', `MaxLevelHealth` = '200702' WHERE `entry` = '21626';
-- Dalliah the Doomsayer (20885,21590) (18/24H)(8.5/8.5D) - 132840/177120
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '20885';
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '221400', `MaxLevelHealth` = '221400' WHERE `entry` = '21590';
-- Wrath-Scryer Soccothrates (20886,21624) (18/24H)(8/15D) - 132840/178220
UPDATE `creature_template` SET `HealthMultiplier` = '23', `MinLevelHealth` = '169740', `MaxLevelHealth` = '169740' WHERE `entry` = '20886';
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '221400', `MaxLevelHealth` = '221400' WHERE `entry` = '21624';
-- Harbinger Skyriss (20912,21599) (25/33H)(4/8D) - 147575/194799
UPDATE `creature_template` SET `HealthMultiplier` = '33', `MinLevelHealth` = '194799', `MaxLevelHealth` = '194799' WHERE `entry` = '20912';
UPDATE `creature_template` SET `HealthMultiplier` = '42', `MinLevelHealth` = '247926', `MaxLevelHealth` = '247926' WHERE `entry` = '21599';

-- =====================================================================================================
-- Magisters' Terrace
-- =====================================================================================================

-- Selin Fireheart (24723,25562) (27.5/37.5H)(3.5/13D) - 164000/215000
UPDATE `creature_template` SET `HealthMultiplier` = '44', `MinLevelHealth` = '252736', `MaxLevelHealth` = '252736' WHERE `entry` = '25562';
-- Vexallus (24744,25573) (25/34H)(8/14.5D) - 143600/195296
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '229760', `MaxLevelHealth` = '229760' WHERE `entry` = '25573';
-- Kael'thas Sunstrider (24664,24857) (30/40H)(4.75/13D) - 177090/236120
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150' WHERE `entry` = '24857';

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.2, `MinLevelHealth` = `MinLevelHealth` * 1.2, `MaxLevelHealth` = `MaxLevelHealth` * 1.2 WHERE `entry` IN (
24722,25552, -- Fel Crystal 6986/13972

24560,25560, -- Priestess Delrissa (Priest) (4.4/6H)(2.75/13D) - 24591.6/33534
24553,25541, -- Apoko (Shaman) (3.85/5.25H)(4/12D) - 21518/29342
24554,25550, -- Eramas Brightblaze (Monk) (2.89/4H)(10/17D) - 20190/27944
24555,25555, -- Garaxxas (Hunter) (3.85/5.25H)(6/13.5D) - 21518/29342
24552,25564, -- Sliver (Ravager) (1.65/2.25H)(5/12D) - 11527/15719
24556,25579, -- Zelfan (Engineer) (2.89/4H)(3/10D) - 16152/22356
24557,25556, -- Kagani Nightstrike (Rogue) (2.89/4H)(5/16D) - 20190/27944
24558,25549, -- Ellrys Duskhallow (Warlock) (2.89/4H)(4/12D) - 16152/22356
24656,25553, -- Fizzle (Imp) (0.4/0.36H)(1.75/3D) - 1956/1760
24559,25574, -- Warlord Salaris (Warrior) (3.85/5.25H)(4.2/8D) - 26896/36677
24561,25578, -- Yazzai (Mage) (3.85/5.25H)(4/12D) - 21518/29342

24674, -- Phoenix (5H/1D) - 34930
24675, -- Phoenix Egg (1H/1D) - 6986

24683,25568, -- Sunblade Mage Guard - 20232/26939
24684,25565, -- Sunblade Blood Knight - 20232/26939
24685,25569, -- Sunblade Magister - 16208/22356
24686,25572, -- Sunblade Warlock - 16767/22356
24687,25570, -- Sunblade Physician - 16767/22356
24688,25577, -- Wretched Skuller - 16903/23664
24689,25575, -- Wretched Bruiser - 16903/23664
24690,25576, -- Wretched Husk - 13523/18932
24696,25547, -- Coilskar Witch - 17232/22976
24697,25563, -- Sister of Torment - 17232/22976
24698,25551, -- Ethereum Smuggler - 21543/28724
24761,25545, -- Brightscale Wyrm - 4471/5868
24762,25567, -- Sunblade Keeper - 15686/21636
24777,25571, -- Sunblade Sentinel - 86172/143620
24815,25566 -- Sunblade Imp - 1843/4401
);

--															=============================
--															=====   Pre 2.1 Tuning  =====
--															=============================
/*
-- =====================================================================================================
-- Karazhan 2.0
-- =====================================================================================================

-- Attumen the Huntsman (15550)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '80.0', `MinLevelHealth` = '607040', `MaxLevelHealth` = '607040', `DamageMultiplier` = '24.0' WHERE `entry` = '15550';
-- Midnight (16151)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '80.0', `MinLevelHealth` = '607040', `MaxLevelHealth` = '607040', `DamageMultiplier` = '19.0' WHERE `entry` = '16151';
-- Attumen the Huntsman (16152)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '80.0', `MinLevelHealth` = '607040', `MaxLevelHealth` = '607040', `DamageMultiplier` = '24.0' WHERE `entry` = '16152';
-- Moroes (15687)+ 60% (17D)
UPDATE `creature_template` SET `HealthMultiplier` = '80.0', `MinLevelHealth` = '607040', `MaxLevelHealth` = '607040', `DamageMultiplier` = '27.0' WHERE `entry` = '15687';
-- Lady Catriona Von'Indi (19872)
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `MinLevelHealth` = '162081', `MaxLevelHealth` = '162081' WHERE `entry` = '19872';
-- Lord Crispin Ference (19873)
UPDATE `creature_template` SET `HealthMultiplier` = '23.5', `MinLevelHealth` = '164171', `MaxLevelHealth` = '164171' WHERE `entry` = '19873';
-- Baron Rafe Dreuger (19874)
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `MinLevelHealth` = '162081', `MaxLevelHealth` = '162081' WHERE `entry` = '19874';
-- Baroness Dorothea Millstipe (19875)
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `MinLevelHealth` = '162081', `MaxLevelHealth` = '162081' WHERE `entry` = '19875';
-- Lord Robin Daris (19876)
UPDATE `creature_template` SET `HealthMultiplier` = '23.5', `MinLevelHealth` = '164171', `MaxLevelHealth` = '164171' WHERE `entry` = '19876';
-- Lady Keira Berrybuck (17007)
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `MinLevelHealth` = '167670', `MaxLevelHealth` = '167670' WHERE `entry` = '17007';
-- Maiden of Virtue (16457)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '112.0', `MinLevelHealth` = '679840', `MaxLevelHealth` = '679840', `DamageMultiplier` = '38.0' WHERE `entry` = '16457';
-- Dorothee (17535)+ 10
UPDATE `creature_template` SET `HealthMultiplier` = '35.0', `MinLevelHealth` = '212450', `MaxLevelHealth` = '212450' WHERE `entry` = '17535';
-- Tito (17548)+ 10
UPDATE `creature_template` SET `HealthMultiplier` = '15.0', `MinLevelHealth` = '104790', `MaxLevelHealth` = '104790' WHERE `entry` = '17548';
-- Roar (17546)+ 10
UPDATE `creature_template` SET `HealthMultiplier` = '25.0', `MinLevelHealth` = '184500', `MaxLevelHealth` = '184500' WHERE `entry` = '17546';
-- Strawman (17543)+ 10
UPDATE `creature_template` SET `HealthMultiplier` = '25.0', `MinLevelHealth` = '184500', `MaxLevelHealth` = '184500' WHERE `entry` = '17543';
-- Tinhead (17547)+ 10
UPDATE `creature_template` SET `HealthMultiplier` = '25.0', `MinLevelHealth` = '184500', `MaxLevelHealth` = '184500' WHERE `entry` = '17547';
-- The Crone (18168)+ 20
UPDATE `creature_template` SET `HealthMultiplier` = '45.0', `MinLevelHealth` = '273150', `MaxLevelHealth` = '273150' WHERE `entry` = '18168';
-- Romulo (17533)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '341460', `MaxLevelHealth` = '341460', `DamageMultiplier` = '22.0' WHERE `entry` = '17533';
-- Julianne (17534)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '54', `MinLevelHealth` = '327780', `MaxLevelHealth` = '327780', `DamageMultiplier` = '20.0' WHERE `entry` = '17534';
-- The Big Bad Wolf (17521)+ 100% (24D)
UPDATE `creature_template` SET `HealthMultiplier` = '100.0', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '32.0' WHERE `entry` = '17521';
-- The Curator (15691)+ 20% (22D)
UPDATE `creature_template` SET `HealthMultiplier` = '226.8', `MinLevelHealth` = '1376676', `MaxLevelHealth` = '1376676', `DamageMultiplier` = '30.0' WHERE `entry` = '15691';
-- Terestian Illhoof (15688)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '160.0', `MinLevelHealth` = '1117760', `MaxLevelHealth` = '1117760', `DamageMultiplier` = '26.0' WHERE `entry` = '15688';
-- Kil'rek (17229)
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `MinLevelHealth` = '63570', `MaxLevelHealth` = '63570', `DamageMultiplier` = '26.0' WHERE `entry` = '17229';
-- Fiendish Imp (17267)
UPDATE `creature_template` SET `HealthMultiplier` = '1.2', `MinLevelHealth` = '5680', `MaxLevelHealth` = '5868', `DamageMultiplier` = '2.0' WHERE `entry` = '17267';
-- Shade of Aran (16524)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '256.0', `MinLevelHealth` = '1359616', `MaxLevelHealth` = '1359616', `DamageMultiplier` = '15.0' WHERE `entry` = '16524';
-- Conjured Elemental (17167)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `DamageMultiplier` = '7.0' WHERE `entry` = '17167';
-- Prince Malchezaar (15690)+ 60% (23D)
UPDATE `creature_template` SET `HealthMultiplier` = '240.0', `MinLevelHealth` = '1821120', `MaxLevelHealth` = '1821120', `DamageMultiplier` = '41.0' WHERE `entry` = '15690';
-- Prince Malchezaar's Axes (17650)+ (2D)
UPDATE `creature_template` SET `DamageMultiplier` = '3.0' WHERE `entry` = '17650';
-- Netherspite (15689)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '240.0', `MinLevelHealth` = '1341360', `MaxLevelHealth` = '1341360', `DamageMultiplier` = '29.0' WHERE `entry` = '15689';
-- Nightbane (17225)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '280.0', `MinLevelHealth` = '2124640', `MaxLevelHealth` = '2124640', `DamageMultiplier` = '38.0' WHERE `entry` = '17225';
-- Restless Skeleton (17261)+ 0% (7D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `MinLevelHealth` = '16355', `MaxLevelHealth` = '17465', `DamageMultiplier` = '7.0' WHERE `entry` = '17261';
-- Hyakiss the Lurker (16179)
UPDATE `creature_template` SET `HealthMultiplier` = '40.0', `MinLevelHealth` = '303520', `MaxLevelHealth` = '303520', `DamageMultiplier` = '20.0' WHERE `entry` = '16179';
-- Shadikith the Glider (16180)
UPDATE `creature_template` SET `HealthMultiplier` = '40.0', `MinLevelHealth` = '303520', `MaxLevelHealth` = '303520', `DamageMultiplier` = '29.0' WHERE `entry` = '16180';
-- Rokad the Ravager (16181)
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `MinLevelHealth` = '227640', `MaxLevelHealth` = '227640', `DamageMultiplier` = '40.0' WHERE `entry` = '16181';

-- ============================
-- Karazhan Trash 2.0
-- ============================

-- Spectral Charger (15547)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '19.0' WHERE `entry` = '15547';
-- Spectral Stallion (15548)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '61038', `MaxLevelHealth` = '61038', `DamageMultiplier` = '8.5' WHERE `entry` = '15548';
-- Spectral Stable Hand (15551)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '47506', `MaxLevelHealth` = '47506', `DamageMultiplier` = '11.5' WHERE `entry` = '15551';
-- Coldmist Stalker (16170)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `DamageMultiplier` = '10.0' WHERE `entry` = '16170';
-- Coldmist Widow (16171)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '68219', `MaxLevelHealth` = '68219', `DamageMultiplier` = '16.0' WHERE `entry` = '16171';
-- Shadowbat (16173)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `MinLevelHealth` = '33805', `MaxLevelHealth` = '34930', `DamageMultiplier` = '10.0' WHERE `entry` = '16173';
-- Greater Shadowbat (16174)
UPDATE `creature_template` SET `HealthMultiplier` = '12.0', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '20.0' WHERE `entry` = '16174';
-- Vampiric Shadowbat (16175)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '68219', `MaxLevelHealth` = '68219', `DamageMultiplier` = '16.0' WHERE `entry` = '16175';
-- Shadowbeast (16176)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '57468', `MaxLevelHealth` = '59381', `DamageMultiplier` = '14.0' WHERE `entry` = '16176';
-- Dreadbeast (16177)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '68219', `MaxLevelHealth` = '68219', `DamageMultiplier` = '20.0' WHERE `entry` = '16177';
-- Phase Hound (16178)
UPDATE `creature_template` SET `HealthMultiplier` = '5.5', `MinLevelHealth` = '38423', `MaxLevelHealth` = '38423', `DamageMultiplier` = '12.0' WHERE `entry` = '16178';
-- Spectral Apprentice (16389)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '62874', `MaxLevelHealth` = '62874', `DamageMultiplier` = '14.0' WHERE `entry` = '16389';
-- Phantom Attendant (16406)
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `MinLevelHealth` = '34464', `MaxLevelHealth` = '34464', `DamageMultiplier` = '14.0' WHERE `entry` = '16406';
-- Spectral Servant (16407)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '57468', `MaxLevelHealth` = '59381', `DamageMultiplier` = '14.0' WHERE `entry` = '16407';
-- Phantom Valet (16408)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `DamageMultiplier` = '27.0' WHERE `entry` = '16408';
-- Spectral Retainer (16410)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '81180', `MaxLevelHealth` = '81180', `DamageMultiplier` = '24.0' WHERE `entry` = '16410';
-- Spectral Chef (16411)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '61038', `MaxLevelHealth` = '61038', `DamageMultiplier` = '23.0' WHERE `entry` = '16411';
-- Ghostly Baker (16412)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `DamageMultiplier` = '14.0' WHERE `entry` = '16412';
-- Ghostly Steward (16414)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '61039', `MaxLevelHealth` = '61039', `DamageMultiplier` = '14.0' WHERE `entry` = '16414';
-- Skeletal Waiter (16415)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `DamageMultiplier` = '17.0' WHERE `entry` = '16415';
-- Spectral Sentry (16424)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `DamageMultiplier` = '17.0' WHERE `entry` = '16424';
-- Phantom Guardsman (16425)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `DamageMultiplier` = '14.0' WHERE `entry` = '16425';
-- Wanton Hostess (16459)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '17.0' WHERE `entry` = '16459';
-- Night Mistress (16460)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '63184', `MaxLevelHealth` = '63184', `DamageMultiplier` = '17.0' WHERE `entry` = '16460';
-- Concubine (16461)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '17.0' WHERE `entry` = '16461';
-- Spectral Patron (16468)
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `MinLevelHealth` = '27944', `MaxLevelHealth` = '27944', `DamageMultiplier` = '8.0' WHERE `entry` = '16468';
-- Ghostly Philanthropist (16470)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '53127', `MaxLevelHealth` = '53127', `DamageMultiplier` = '15.0' WHERE `entry` = '16470';
-- Skeletal Usher (16471)
UPDATE `creature_template` SET `HealthMultiplier` = '16.0', `MinLevelHealth` = '94448', `MaxLevelHealth` = '94448', `DamageMultiplier` = '27.0' WHERE `entry` = '16471';
-- Phantom Stagehand (16472)
UPDATE `creature_template` SET `HealthMultiplier` = '9.3', `MinLevelHealth` = '66783', `MaxLevelHealth` = '66783', `DamageMultiplier` = '31.0' WHERE `entry` = '16472';
-- Spectral Performer (16473)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '24.0' WHERE `entry` = '16473';
-- Ghastly Haunt (16481)
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `MinLevelHealth` = '95940', `MaxLevelHealth` = '95940', `DamageMultiplier` = '35.0' WHERE `entry` = '16481';
-- Trapped Soul (16482)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '56078', `MaxLevelHealth` = '56078', `DamageMultiplier` = '26.0' WHERE `entry` = '16482';
-- Arcane Watchman (16485)
UPDATE `creature_template` SET `HealthMultiplier` = '16.0', `MinLevelHealth` = '118080', `MaxLevelHealth` = '118080', `DamageMultiplier` = '25.0' WHERE `entry` = '16485';
-- Arcane Anomaly (16488)
UPDATE `creature_template` SET `HealthMultiplier` = '0.4', `MinLevelHealth` = '2010', `MaxLevelHealth` = '2010', `DamageMultiplier` = '18.0' WHERE `entry` = '16488';
-- Chaotic Sentience (16489)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '69860', `MaxLevelHealth` = '71810', `DamageMultiplier` = '18.0' WHERE `entry` = '16489';
-- Mana Feeder (16491)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `DamageMultiplier` = '8.0' WHERE `entry` = '16491';
-- Syphoner (16492)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `MinLevelHealth` = '16767', `MaxLevelHealth` = '16767', `DamageMultiplier` = '8.0' WHERE `entry` = '16492';
-- Arcane Protector (16504)
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980', `DamageMultiplier` = '36.0' WHERE `entry` = '16504';
-- Spell Shade (16525)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `DamageMultiplier` = '15.0' WHERE `entry` = '16525';
-- Sorcerous Shade (16526)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '64933', `MaxLevelHealth` = '64933', `DamageMultiplier` = '15.0' WHERE `entry` = '16526';
-- Magical Horror (16529)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '57440', `MaxLevelHealth` = '57440', `DamageMultiplier` = '14.0' WHERE `entry` = '16529';
-- Mana Warp (16530)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '55888', `MaxLevelHealth` = '55888', `DamageMultiplier` = '8.0' WHERE `entry` = '16530';
-- Homunculus (16539)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `MinLevelHealth` = '27945', `MaxLevelHealth` = '27945', `DamageMultiplier` = '5.0' WHERE `entry` = '16539';
-- Shadow Pillager (16540)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '47224', `MaxLevelHealth` = '47224', `DamageMultiplier` = '10.0' WHERE `entry` = '16540';
-- Ethereal Thief (16544)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `DamageMultiplier` = '27.0' WHERE `entry` = '16544';
-- Ethereal Spellfilcher (16545)
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `MinLevelHealth` = '76739', `MaxLevelHealth` = '76739', `DamageMultiplier` = '28.0' WHERE `entry` = '16545';
-- Fleshbeast (16595)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '66420', `MaxLevelHealth` = '66420', `DamageMultiplier` = '25.0' WHERE `entry` = '16595';
-- Greater Fleshbeast (16596)
UPDATE `creature_template` SET `HealthMultiplier` = '19.0', `MinLevelHealth` = '140220', `MaxLevelHealth` = '140220', `DamageMultiplier` = '40.0' WHERE `entry` = '16596';
-- Phantom Hound (17067)
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972', `DamageMultiplier` = '2.0' WHERE `entry` = '17067';

-- =====================================================================================================
-- Gruul's Lair 2.0
-- =====================================================================================================

-- High King Maulgar (18831)+ 80% (40D)
UPDATE `creature_template` SET `HealthMultiplier` = '180.0', `MinLevelHealth` = '1365840', `MaxLevelHealth` = '1365840', `DamageMultiplier` = '45.0' WHERE `entry` = '18831';
-- Krosh Firehand (18832)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '90.0', `MinLevelHealth` = '546300', `MaxLevelHealth` = '546300', `DamageMultiplier` = '30.0' WHERE `entry` = '18832';
-- Olm the Summoner (18834)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '90.0', `MinLevelHealth` = '546300', `MaxLevelHealth` = '546300', `DamageMultiplier` = '32.0' WHERE `entry` = '18834';
-- Kiggler the Crazed (18835)+ 80% (28D)
UPDATE `creature_template` SET `HealthMultiplier` = '90.0', `MinLevelHealth` = '546300', `MaxLevelHealth` = '546300', `DamageMultiplier` = '50.0' WHERE `entry` = '18835';
-- Blindeye the Seer (18836)+ 80% (7D)
UPDATE `creature_template` SET `HealthMultiplier` = '90.0', `MinLevelHealth` = '546300', `MaxLevelHealth` = '546300', `DamageMultiplier` = '25.0' WHERE `entry` = '18836';
-- Wild Fel Stalker (18847)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '60704', `MaxLevelHealth` = '60704', `DamageMultiplier` = '12.0' WHERE `entry` = '18847';
-- Gruul the Dragonkiller (19044)+ 20% (26D)
UPDATE `creature_template` SET `HealthMultiplier` = '759.6', `MinLevelHealth` = '5763845', `MaxLevelHealth` = '5763845', `DamageMultiplier` = '28.0' WHERE `entry` = '19044';

-- ============================
-- Gruul's Lair Trash 2.0
-- ============================

-- Lair Brute (19389)+ 60% &dmg 20%
UPDATE `creature_template` SET `HealthMultiplier` = '64.0', `MinLevelHealth` = '472320', `MaxLevelHealth` = '472320', `DamageMultiplier` = '42.0' WHERE `entry` = '19389';
-- Gronn-Priest (21350)+ 60% &dmg 20%
UPDATE `creature_template` SET `HealthMultiplier` = '67.2', `MinLevelHealth` = '396682', `MaxLevelHealth` = '396682', `DamageMultiplier` = '36.0' WHERE `entry` = '21350';

-- =====================================================================================================
-- Magtheridon's Lair 2.0
-- =====================================================================================================

-- Magtheridon (17257)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '762.0', `MinLevelHealth` = '5782056', `MaxLevelHealth` = '5782056', `DamageMultiplier` = '50.0' WHERE `entry` = '17257';
-- Hellfire Channeler (17256)+ 0% (40H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200' WHERE `entry` = '17256';

-- ============================
-- Magtheridon's Lair Trash
-- ============================

-- Hellfire Warder (18829)+ 0% (32H / 22D)
UPDATE `creature_template` SET `HealthMultiplier` = '50.0', `MinLevelHealth` = '295150', `MaxLevelHealth` = '295150', `DamageMultiplier` = '33.0' WHERE `entry` = '18829';

-- =====================================================================================================
-- World Bosses 2.0
-- =====================================================================================================

-- Doomwalker (17711)+ 50%
UPDATE `creature_template` SET `HealthMultiplier` = '510.0', `MinLevelHealth` = '3869880', `MaxLevelHealth` = '3869880', `DamageMultiplier` = '36.0' WHERE `entry` = '17711';
-- Doom Lord Kazzak (18728)+ 15%
UPDATE `creature_template` SET `HealthMultiplier` = '197.8', `MinLevelHealth` = '1500906', `MaxLevelHealth` = '1500906', `DamageMultiplier` = '90.0' WHERE `entry` = '18728';

-- =====================================================================================================
-- Coilfang Reservoir: Serpentshrine Cavern 2.0
-- =====================================================================================================

-- Hydross the Unstable (21216)+ 50%
UPDATE `creature_template` SET `HealthMultiplier` = '750', `MinLevelHealth` = '5691000', `MaxLevelHealth` = '5691000' WHERE `entry` = '21216';
-- The Lurker Below (21217)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '840', `MinLevelHealth` = '6373920', `MaxLevelHealth` = '6373920', `DamageMultiplier` = '42' WHERE `entry` = '21217';
-- Leotheras the Blind (21215)+ 40% (500H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '840', `MinLevelHealth` = '6373920', `MaxLevelHealth` = '6373920', `DamageMultiplier` = '30' WHERE `entry` = '21215';
-- Fathom-Lord Karathress (21214)+ 50% &dmg 25%
UPDATE `creature_template` SET `HealthMultiplier` = '450', `MinLevelHealth` = '2731500', `MaxLevelHealth` = '2731500', `DamageMultiplier` = '45' WHERE `entry` = '21214';
-- Fathom-Guard Caribdis (21964)+ 50% &dmg 25%
UPDATE `creature_template` SET `HealthMultiplier` = '225', `MinLevelHealth` = '1292400', `MaxLevelHealth` = '1292400', `DamageMultiplier` = '31.25' WHERE `entry` = '21964';
-- Fathom-Guard Tidalvess (21965)+ 50%
UPDATE `creature_template` SET `HealthMultiplier` = '225', `MinLevelHealth` = '1292400', `MaxLevelHealth` = '1292400', `DamageMultiplier` = '42' WHERE `entry` = '21965';
-- Fathom-Guard Sharkkis (21966)+ 50% &dmg 25%
UPDATE `creature_template` SET `HealthMultiplier` = '225', `MinLevelHealth` = '1292400', `MaxLevelHealth` = '1292400', `DamageMultiplier` = '31.25' WHERE `entry` = '21966';
-- Morogrim Tidewalker (21213)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '1050', `MinLevelHealth` = '7967400', `MaxLevelHealth` = '7967400', `DamageMultiplier` = '56' WHERE `entry` = '21213';
-- Tidewalker Lurker (21920)+ (10H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '7.5', `MinLevelHealth` = '53857.5', `MaxLevelHealth` = '53857.5' WHERE `entry` = '21920';
-- Lady Vashj (21212)+ 10% &dmg 25%
UPDATE `creature_template` SET `HealthMultiplier` = '1100', `MinLevelHealth` = '6677000', `MaxLevelHealth` = '6677000', `DamageMultiplier` = '56.25' WHERE `entry` = '21212';
-- Coilfang Elite (22055)+
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '279400', `MaxLevelHealth` = '279400', `DamageMultiplier` = '33' WHERE `entry` = '22055';
-- Toxic Spore Bat (22140)+ (41916)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986' WHERE `entry` = '22140';
-- Coilfang Strider (22056)+
UPDATE `creature_template` SET `HealthMultiplier` = '60', `MinLevelHealth` = '419160', `MaxLevelHealth` = '419160', `DamageMultiplier` = '60' WHERE `entry` = '22056';

-- ============================
-- Coilfang Reservoir: Serpentshrine Cavern Trash 2.0
-- ============================



-- =====================================================================================================
-- Tempest Keep: The Eye 2.0
-- =====================================================================================================

-- Al'ar 19514+ 15%
UPDATE `creature_template` SET `HealthMultiplier` = '517.5', `MinLevelHealth` = '3926790', `MaxLevelHealth` = '3926790', `DamageMultiplier` = '30' WHERE `entry` = '19514';
-- Void Reaver 19516+ 50%
UPDATE `creature_template` SET `HealthMultiplier` = '1125', `MinLevelHealth` = '6828750', `MaxLevelHealth` = '6828750', `DamageMultiplier` = '36' WHERE `entry` = '19516';
-- High Astromancer Solarian 18805+ 50%
UPDATE `creature_template` SET `HealthMultiplier` = '1125', `MinLevelHealth` = '6828750', `MaxLevelHealth` = '6828750', `DamageMultiplier` = '25' WHERE `entry` = '18805';
-- Kael'thas Sunstrider 19622+ 10% (40D)
UPDATE `creature_template` SET `HealthMultiplier` = '1100', `MinLevelHealth` = '6677000', `MaxLevelHealth` = '6677000', `DamageMultiplier` = '50' WHERE `entry` = '19622';

-- ============================
-- Tempest Keep: The Eye Trash 2.0
-- ============================

UPDATE `creature_template` SET `DamageMultiplier` = `DamageMultiplier` * 1.20 WHERE `entry` IN (
20031, -- Bloodwarder Legionnaire 20031 (25H/28D)
20032, -- Bloodwarder Vindicator 20032 (40H/32D)
20033, -- Astromancer 20033 (25H/14D)
20034, -- Star Scryer 20034 (25H/16D)
20035, -- Bloodwarder Marshal 20035 (40H/32D)
20036, -- Bloodwarder Squire 20036 (25H/24D)
20037, -- Tempest Falconer 20037 (17.5H/10D)
-- 20038, -- Phoenix-Hawk Hatchling 20038 (7H/14D) -- aoe trash
20039, -- Phoenix-Hawk 20039 (10H/16D)
20040, -- Crystalcore Devastator 20040 (75H/32D)
20041, -- Crystalcore Sentinel 20041 (40H/28D)
20042, -- Tempest-Smith 20042 (17.5H/13D)
-- 20043, -- Apprentice Star Scryer 20043 (2.8H/10D) -- aoe trash
-- 20044, -- Novice Astromancer 20044 (2.8H/10D) -- aoe trash
20045, -- Nether Scryer 20045 (40H/20D)
20046, -- Astromancer Lord 20046 (40H/20D)
20047, -- Crimson Hand Battle Mage 20047 (17.5H/16D)
20048, -- Crimson Hand Centurion 20048 (17.5H/15D)
20049, -- Crimson Hand Blood Knight 20049 (28H/16D)
20050, -- Crimson Hand Inquisitor 20050 (28H/15D)
20052 -- Crystalcore Mechanic 20052 (25H/20D)
);

-- =====================================================================================================
-- Caverns of Time: Hyjal Summit 2.0
-- Damage + 20%
-- =====================================================================================================

-- Rage Winterchill (17767)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '1260', `MinLevelHealth` = '7648200', `MaxLevelHealth` = '7648200', `DamageMultiplier` = '54' WHERE `entry` = '17767';
-- Anetheron (17808)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '1260', `MinLevelHealth` = '7648200', `MaxLevelHealth` = '7648200', `DamageMultiplier` = '60' WHERE `entry` = '17808';
-- Kaz'rogal (17888)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '980', `MinLevelHealth` = '5948600', `MaxLevelHealth` = '5948600', `DamageMultiplier` = '60' WHERE `entry` = '17888';
-- Azgalor (17842)+ 50%
UPDATE `creature_template` SET `HealthMultiplier` = '1050', `MinLevelHealth` = '6373500', `MaxLevelHealth` = '6373500', `DamageMultiplier` = '102' WHERE `entry` = '17842';
-- Archimonde (17968)+ 80%
UPDATE `creature_template` SET `HealthMultiplier` = '1350', `MinLevelHealth` = '8194500', `MaxLevelHealth` = '8194500', `DamageMultiplier` = '132' WHERE `entry` = '17968';

-- ============================
-- Caverns of Time: Hyjal Summit Trash 2.0
-- ============================

-- Ghoul (17895)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '24', `MinLevelHealth` = '167664', `MaxLevelHealth` = '167664', `DamageMultiplier` = '12' WHERE `entry` = '17895';
-- Crypt Fiend (17897)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '209580', `MaxLevelHealth` = '209580', `DamageMultiplier` = '17.124' WHERE `entry` = '17897';
-- Abomination (17898)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '215430', `MaxLevelHealth` = '215430', `DamageMultiplier` = '20.4' WHERE `entry` = '17898';
-- Shadowy Necromancer (17899)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '26.4', `MinLevelHealth` = '172142', `MaxLevelHealth` = '172142', `DamageMultiplier` = '20.4' WHERE `entry` = '17899';
-- Skeleton Invader (17902)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '6', `MinLevelHealth` = '40566', `MaxLevelHealth` = '40566', `DamageMultiplier` = '5.28' WHERE `entry` = '17902';
-- Skeleton Mage (17903)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '6', `MinLevelHealth` = '28398', `MaxLevelHealth` = '28398', `DamageMultiplier` = '5.4' WHERE `entry` = '17903';
-- Banshee (17905)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '100602', `MaxLevelHealth` = '100602', `DamageMultiplier` = '10.8' WHERE `entry` = '17905';
-- Gargoyle (17906)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '21.6', `MinLevelHealth` = '150898', `MaxLevelHealth` = '150898', `DamageMultiplier` = '15.6' WHERE `entry` = '17906';
-- Frost Wyrm (17907)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '72', `MinLevelHealth` = '531360', `MaxLevelHealth` = '531360', `DamageMultiplier` = '24' WHERE `entry` = '17907';
-- Giant Infernal (17908)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '21.6', `MinLevelHealth` = '155110', `MaxLevelHealth` = '155110', `DamageMultiplier` = '9.6' WHERE `entry` = '17908';
-- Fel Stalker (17916)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '18', `MinLevelHealth` = '125748', `MaxLevelHealth` = '125748', `DamageMultiplier` = '12' WHERE `entry` = '17916';


-- Lady Jaina Proudmoore (17772)
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '273150', `MaxLevelHealth` = '273150' WHERE `entry` = '17772';
-- Thrall (17852)
UPDATE `creature_template` SET `HealthMultiplier` = '45', `MinLevelHealth` = '273150', `MaxLevelHealth` = '273150' WHERE `entry` = '17852';
*/
--															=============================
--															=====   Pre 2.4 Tuning  =====
--															=============================
/*
-- =====================================================================================================
-- Karazhan 2.1
-- =====================================================================================================

-- Attumen the Huntsman (15550)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '65.0', `MinLevelHealth` = '493220', `MaxLevelHealth` = '493220', `DamageMultiplier` = '24.0' WHERE `entry` = '15550';
-- Midnight (16151)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '65.0', `MinLevelHealth` = '493220', `MaxLevelHealth` = '493220', `DamageMultiplier` = '19.0' WHERE `entry` = '16151';
-- Attumen the Huntsman (16152)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '65.0', `MinLevelHealth` = '493220', `MaxLevelHealth` = '493220', `DamageMultiplier` = '24.0' WHERE `entry` = '16152';
-- Moroes (15687)+ 30% (17D)
UPDATE `creature_template` SET `HealthMultiplier` = '65.0', `MinLevelHealth` = '493220', `MaxLevelHealth` = '493220', `DamageMultiplier` = '22.0' WHERE `entry` = '15687';
-- Lady Catriona Von'Indi (19872)
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `MinLevelHealth` = '162081', `MaxLevelHealth` = '162081' WHERE `entry` = '19872';
-- Lord Crispin Ference (19873)
UPDATE `creature_template` SET `HealthMultiplier` = '23.5', `MinLevelHealth` = '164171', `MaxLevelHealth` = '164171' WHERE `entry` = '19873';
-- Baron Rafe Dreuger (19874)
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `MinLevelHealth` = '162081', `MaxLevelHealth` = '162081' WHERE `entry` = '19874';
-- Baroness Dorothea Millstipe (19875)
UPDATE `creature_template` SET `HealthMultiplier` = '29.0', `MinLevelHealth` = '162081', `MaxLevelHealth` = '162081' WHERE `entry` = '19875';
-- Lord Robin Daris (19876)
UPDATE `creature_template` SET `HealthMultiplier` = '23.5', `MinLevelHealth` = '164171', `MaxLevelHealth` = '164171' WHERE `entry` = '19876';
-- Lady Keira Berrybuck (17007)
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `MinLevelHealth` = '167670', `MaxLevelHealth` = '167670' WHERE `entry` = '17007';
-- Maiden of Virtue (16457)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '91.0', `MinLevelHealth` = '552370', `MaxLevelHealth` = '552370', `DamageMultiplier` = '38.0' WHERE `entry` = '16457';
-- Dorothee (17535)+ 5
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `MinLevelHealth` = '182100', `MaxLevelHealth` = '182100' WHERE `entry` = '17535';
-- Tito (17548)+ 5
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '69860', `MaxLevelHealth` = '69860' WHERE `entry` = '17548';
-- Roar (17546)+ 5
UPDATE `creature_template` SET `HealthMultiplier` = '20.0', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '17546';
-- Strawman (17543)+ 5
UPDATE `creature_template` SET `HealthMultiplier` = '20.0', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '17543';
-- Tinhead (17547)+ 5
UPDATE `creature_template` SET `HealthMultiplier` = '20.0', `MinLevelHealth` = '147600', `MaxLevelHealth` = '147600' WHERE `entry` = '17547';
-- The Crone (18168)+ 15
UPDATE `creature_template` SET `HealthMultiplier` = '40.0', `MinLevelHealth` = '242800', `MaxLevelHealth` = '242800' WHERE `entry` = '18168';
-- Romulo (17533)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '303520', `MaxLevelHealth` = '303520', `DamageMultiplier` = '22.0' WHERE `entry` = '17533';
-- Julianne (17534)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '48', `MinLevelHealth` = '291360', `MaxLevelHealth` = '291360', `DamageMultiplier` = '20.0' WHERE `entry` = '17534';
-- The Big Bad Wolf (17521)+ 100% (24D)
UPDATE `creature_template` SET `HealthMultiplier` = '100.0', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '28.0' WHERE `entry` = '17521';
-- The Curator (15691)+ 10% (22D)
UPDATE `creature_template` SET `HealthMultiplier` = '207.9', `MinLevelHealth` = '1261953', `MaxLevelHealth` = '1261953', `DamageMultiplier` = '28.0' WHERE `entry` = '15691';
-- Terestian Illhoof (15688)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '130.0', `MinLevelHealth` = '908180', `MaxLevelHealth` = '908180', `DamageMultiplier` = '26.0' WHERE `entry` = '15688';
-- Kil'rek (17229)
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `MinLevelHealth` = '63570', `MaxLevelHealth` = '63570', `DamageMultiplier` = '26.0' WHERE `entry` = '17229';
-- Fiendish Imp (17267)
UPDATE `creature_template` SET `HealthMultiplier` = '1.0', `MinLevelHealth` = '4733', `MaxLevelHealth` = '4890', `DamageMultiplier` = '2.0' WHERE `entry` = '17267';
-- Shade of Aran (16524)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '208.0', `MinLevelHealth` = '1104688', `MaxLevelHealth` = '1104688', `DamageMultiplier` = '15.0' WHERE `entry` = '16524';
-- Conjured Elemental (17167)
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `DamageMultiplier` = '7.0' WHERE `entry` = '17167';
-- Prince Malchezaar (15690)+ 30% (23D)
UPDATE `creature_template` SET `HealthMultiplier` = '195.0', `MinLevelHealth` = '1479660', `MaxLevelHealth` = '1479660', `DamageMultiplier` = '39.0' WHERE `entry` = '15690';
-- Prince Malchezaar's Axes (17650)+ (2D)
UPDATE `creature_template` SET `DamageMultiplier` = '2.5' WHERE `entry` = '17650';
-- Netherspite (15689)+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '220.0', `MinLevelHealth` = '1229580', `MaxLevelHealth` = '1229580', `DamageMultiplier` = '29.0' WHERE `entry` = '15689';
-- Nightbane (17225)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '227.5', `MinLevelHealth` = '1726270', `MaxLevelHealth` = '1726270', `DamageMultiplier` = '38.0' WHERE `entry` = '17225';
-- Restless Skeleton (17261)+ 0% (7D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `MinLevelHealth` = '16355', `MaxLevelHealth` = '17465', `DamageMultiplier` = '7.0' WHERE `entry` = '17261';
-- Hyakiss the Lurker (16179)
UPDATE `creature_template` SET `HealthMultiplier` = '40.0', `MinLevelHealth` = '303520', `MaxLevelHealth` = '303520', `DamageMultiplier` = '20.0' WHERE `entry` = '16179';
-- Shadikith the Glider (16180)
UPDATE `creature_template` SET `HealthMultiplier` = '40.0', `MinLevelHealth` = '303520', `MaxLevelHealth` = '303520', `DamageMultiplier` = '29.0' WHERE `entry` = '16180';
-- Rokad the Ravager (16181)
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `MinLevelHealth` = '227640', `MaxLevelHealth` = '227640', `DamageMultiplier` = '40.0' WHERE `entry` = '16181';

-- ============================
-- Karazhan Trash 2.1
-- ============================

-- Spectral Charger (15547)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '19.0' WHERE `entry` = '15547';
-- Spectral Stallion (15548)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '61038', `MaxLevelHealth` = '61038', `DamageMultiplier` = '8.5' WHERE `entry` = '15548';
-- Spectral Stable Hand (15551)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '47506', `MaxLevelHealth` = '47506', `DamageMultiplier` = '11.5' WHERE `entry` = '15551';
-- Coldmist Stalker (16170)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930', `DamageMultiplier` = '10.0' WHERE `entry` = '16170';
-- Coldmist Widow (16171)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '68219', `MaxLevelHealth` = '68219', `DamageMultiplier` = '16.0' WHERE `entry` = '16171';
-- Shadowbat (16173)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `MinLevelHealth` = '33805', `MaxLevelHealth` = '34930', `DamageMultiplier` = '10.0' WHERE `entry` = '16173';
-- Greater Shadowbat (16174)
UPDATE `creature_template` SET `HealthMultiplier` = '12.0', `MinLevelHealth` = '86172', `MaxLevelHealth` = '86172', `DamageMultiplier` = '20.0' WHERE `entry` = '16174';
-- Vampiric Shadowbat (16175)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '68219', `MaxLevelHealth` = '68219', `DamageMultiplier` = '16.0' WHERE `entry` = '16175';
-- Shadowbeast (16176)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '57468', `MaxLevelHealth` = '59381', `DamageMultiplier` = '14.0' WHERE `entry` = '16176';
-- Dreadbeast (16177)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '68219', `MaxLevelHealth` = '68219', `DamageMultiplier` = '20.0' WHERE `entry` = '16177';
-- Phase Hound (16178)
UPDATE `creature_template` SET `HealthMultiplier` = '5.5', `MinLevelHealth` = '38423', `MaxLevelHealth` = '38423', `DamageMultiplier` = '12.0' WHERE `entry` = '16178';
-- Spectral Apprentice (16389)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '62874', `MaxLevelHealth` = '62874', `DamageMultiplier` = '14.0' WHERE `entry` = '16389';
-- Phantom Attendant (16406)
UPDATE `creature_template` SET `HealthMultiplier` = '6.0', `MinLevelHealth` = '34464', `MaxLevelHealth` = '34464', `DamageMultiplier` = '14.0' WHERE `entry` = '16406';
-- Spectral Servant (16407)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '57468', `MaxLevelHealth` = '59381', `DamageMultiplier` = '14.0' WHERE `entry` = '16407';
-- Phantom Valet (16408)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `DamageMultiplier` = '27.0' WHERE `entry` = '16408';
-- Spectral Retainer (16410)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '81180', `MaxLevelHealth` = '81180', `DamageMultiplier` = '24.0' WHERE `entry` = '16410';
-- Spectral Chef (16411)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '61038', `MaxLevelHealth` = '61038', `DamageMultiplier` = '23.0' WHERE `entry` = '16411';
-- Ghostly Baker (16412)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `DamageMultiplier` = '14.0' WHERE `entry` = '16412';
-- Ghostly Steward (16414)
UPDATE `creature_template` SET `HealthMultiplier` = '8.5', `MinLevelHealth` = '61039', `MaxLevelHealth` = '61039', `DamageMultiplier` = '14.0' WHERE `entry` = '16414';
-- Skeletal Waiter (16415)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `DamageMultiplier` = '17.0' WHERE `entry` = '16415';
-- Spectral Sentry (16424)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `DamageMultiplier` = '17.0' WHERE `entry` = '16424';
-- Phantom Guardsman (16425)
UPDATE `creature_template` SET `HealthMultiplier` = '7.0', `MinLevelHealth` = '50267', `MaxLevelHealth` = '50267', `DamageMultiplier` = '14.0' WHERE `entry` = '16425';
-- Wanton Hostess (16459)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '17.0' WHERE `entry` = '16459';
-- Night Mistress (16460)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '63184', `MaxLevelHealth` = '63184', `DamageMultiplier` = '17.0' WHERE `entry` = '16460';
-- Concubine (16461)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '17.0' WHERE `entry` = '16461';
-- Spectral Patron (16468)
UPDATE `creature_template` SET `HealthMultiplier` = '4.0', `MinLevelHealth` = '27944', `MaxLevelHealth` = '27944', `DamageMultiplier` = '8.0' WHERE `entry` = '16468';
-- Ghostly Philanthropist (16470)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '53127', `MaxLevelHealth` = '53127', `DamageMultiplier` = '15.0' WHERE `entry` = '16470';
-- Skeletal Usher (16471)
UPDATE `creature_template` SET `HealthMultiplier` = '16.0', `MinLevelHealth` = '94448', `MaxLevelHealth` = '94448', `DamageMultiplier` = '27.0' WHERE `entry` = '16471';
-- Phantom Stagehand (16472)
UPDATE `creature_template` SET `HealthMultiplier` = '9.3', `MinLevelHealth` = '66783', `MaxLevelHealth` = '66783', `DamageMultiplier` = '31.0' WHERE `entry` = '16472';
-- Spectral Performer (16473)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '78991', `MaxLevelHealth` = '78991', `DamageMultiplier` = '24.0' WHERE `entry` = '16473';
-- Ghastly Haunt (16481)
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `MinLevelHealth` = '95940', `MaxLevelHealth` = '95940', `DamageMultiplier` = '35.0' WHERE `entry` = '16481';
-- Trapped Soul (16482)
UPDATE `creature_template` SET `HealthMultiplier` = '9.5', `MinLevelHealth` = '56078', `MaxLevelHealth` = '56078', `DamageMultiplier` = '26.0' WHERE `entry` = '16482';
-- Arcane Watchman (16485)
UPDATE `creature_template` SET `HealthMultiplier` = '16.0', `MinLevelHealth` = '118080', `MaxLevelHealth` = '118080', `DamageMultiplier` = '25.0' WHERE `entry` = '16485';
-- Arcane Anomaly (16488)
UPDATE `creature_template` SET `HealthMultiplier` = '0.4', `MinLevelHealth` = '2010', `MaxLevelHealth` = '2010', `DamageMultiplier` = '18.0' WHERE `entry` = '16488';
-- Chaotic Sentience (16489)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '69860', `MaxLevelHealth` = '71810', `DamageMultiplier` = '18.0' WHERE `entry` = '16489';
-- Mana Feeder (16491)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `MinLevelHealth` = '20958', `MaxLevelHealth` = '20958', `DamageMultiplier` = '8.0' WHERE `entry` = '16491';
-- Syphoner (16492)
UPDATE `creature_template` SET `HealthMultiplier` = '3.0', `MinLevelHealth` = '16767', `MaxLevelHealth` = '16767', `DamageMultiplier` = '8.0' WHERE `entry` = '16492';
-- Arcane Protector (16504)
UPDATE `creature_template` SET `HealthMultiplier` = '21.0', `MinLevelHealth` = '154980', `MaxLevelHealth` = '154980', `DamageMultiplier` = '36.0' WHERE `entry` = '16504';
-- Spell Shade (16525)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '64629', `MaxLevelHealth` = '64629', `DamageMultiplier` = '15.0' WHERE `entry` = '16525';
-- Sorcerous Shade (16526)
UPDATE `creature_template` SET `HealthMultiplier` = '11.0', `MinLevelHealth` = '64933', `MaxLevelHealth` = '64933', `DamageMultiplier` = '15.0' WHERE `entry` = '16526';
-- Magical Horror (16529)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '57440', `MaxLevelHealth` = '57440', `DamageMultiplier` = '14.0' WHERE `entry` = '16529';
-- Mana Warp (16530)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '55888', `MaxLevelHealth` = '55888', `DamageMultiplier` = '8.0' WHERE `entry` = '16530';
-- Homunculus (16539)
UPDATE `creature_template` SET `HealthMultiplier` = '5.0', `MinLevelHealth` = '27945', `MaxLevelHealth` = '27945', `DamageMultiplier` = '5.0' WHERE `entry` = '16539';
-- Shadow Pillager (16540)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '47224', `MaxLevelHealth` = '47224', `DamageMultiplier` = '10.0' WHERE `entry` = '16540';
-- Ethereal Thief (16544)
UPDATE `creature_template` SET `HealthMultiplier` = '10.0', `MinLevelHealth` = '73800', `MaxLevelHealth` = '73800', `DamageMultiplier` = '27.0' WHERE `entry` = '16544';
-- Ethereal Spellfilcher (16545)
UPDATE `creature_template` SET `HealthMultiplier` = '13.0', `MinLevelHealth` = '76739', `MaxLevelHealth` = '76739', `DamageMultiplier` = '28.0' WHERE `entry` = '16545';
-- Fleshbeast (16595)
UPDATE `creature_template` SET `HealthMultiplier` = '9.0', `MinLevelHealth` = '66420', `MaxLevelHealth` = '66420', `DamageMultiplier` = '25.0' WHERE `entry` = '16595';
-- Greater Fleshbeast (16596)
UPDATE `creature_template` SET `HealthMultiplier` = '19.0', `MinLevelHealth` = '140220', `MaxLevelHealth` = '140220', `DamageMultiplier` = '40.0' WHERE `entry` = '16596';
-- Phantom Hound (17067)
UPDATE `creature_template` SET `HealthMultiplier` = '2.0', `MinLevelHealth` = '13972', `MaxLevelHealth` = '13972', `DamageMultiplier` = '2.0' WHERE `entry` = '17067';

-- =====================================================================================================
-- Gruul's Lair 2.1
-- =====================================================================================================

-- High King Maulgar (18831)+ 40% (40D)
UPDATE `creature_template` SET `HealthMultiplier` = '140.0', `MinLevelHealth` = '1062320', `MaxLevelHealth` = '1062320', `DamageMultiplier` = '45.0' WHERE `entry` = '18831';
-- Krosh Firehand (18832)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '70.0', `MinLevelHealth` = '424900', `MaxLevelHealth` = '424900', `DamageMultiplier` = '30.0' WHERE `entry` = '18832';
-- Olm the Summoner (18834)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '70.0', `MinLevelHealth` = '424900', `MaxLevelHealth` = '424900', `DamageMultiplier` = '32.0' WHERE `entry` = '18834';
-- Kiggler the Crazed (18835)+ 40% (28D)
UPDATE `creature_template` SET `HealthMultiplier` = '70.0', `MinLevelHealth` = '424900', `MaxLevelHealth` = '424900', `DamageMultiplier` = '50.0' WHERE `entry` = '18835';
-- Blindeye the Seer (18836)+ 40% (7D)
UPDATE `creature_template` SET `HealthMultiplier` = '70.0', `MinLevelHealth` = '424900', `MaxLevelHealth` = '424900', `DamageMultiplier` = '25.0' WHERE `entry` = '18836';
-- Wild Fel Stalker (18847)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '60704', `MaxLevelHealth` = '60704', `DamageMultiplier` = '12.0' WHERE `entry` = '18847';
-- Gruul the Dragonkiller (19044)+ 10% (26D)
UPDATE `creature_template` SET `HealthMultiplier` = '696.3', `MinLevelHealth` = '5283524', `MaxLevelHealth` = '5283524', `DamageMultiplier` = '27.0' WHERE `entry` = '19044';

-- ============================
-- Gruul's Lair Trash 2.1
-- ============================

-- Lair Brute (19389)+ 30% %dmg 10%
UPDATE `creature_template` SET `HealthMultiplier` = '52.0', `MinLevelHealth` = '383760', `MaxLevelHealth` = '383760', `DamageMultiplier` = '38.5' WHERE `entry` = '19389';
-- Gronn-Priest (21350)+ 30% %dmg 10%
UPDATE `creature_template` SET `HealthMultiplier` = '54.6', `MinLevelHealth` = '322304', `MaxLevelHealth` = '322304', `DamageMultiplier` = '33.0' WHERE `entry` = '21350';

-- =====================================================================================================
-- Magtheridon's Lair 2.1
-- =====================================================================================================

-- Magtheridon (17257)+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '698.5', `MinLevelHealth` = '5300218', `MaxLevelHealth` = '5300218', `DamageMultiplier` = '50.0' WHERE `entry` = '17257';
-- Hellfire Channeler (17256)- 15% (40H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '51', `MinLevelHealth` = '309570', `MaxLevelHealth` = '309570' WHERE `entry` = '17256';

-- ============================
-- Magtheridon's Lair Trash 2.1
-- ============================

-- Hellfire Warder (18829)- 15% (32H / 22D)
UPDATE `creature_template` SET `HealthMultiplier` = '42.5', `MinLevelHealth` = '250878', `MaxLevelHealth` = '250878', `DamageMultiplier` = '33.0' WHERE `entry` = '18829';

-- =====================================================================================================
-- World Bosses 2.1
-- =====================================================================================================

-- Doomwalker (17711)+ 35%
UPDATE `creature_template` SET `HealthMultiplier` = '459.0', `MinLevelHealth` = '3482892', `MaxLevelHealth` = '3482892', `DamageMultiplier` = '36.0' WHERE `entry` = '17711';
-- Doom Lord Kazzak (18728)+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '189.2', `MinLevelHealth` = '1435650', `MaxLevelHealth` = '1435650', `DamageMultiplier` = '90.0' WHERE `entry` = '18728';

-- =====================================================================================================
-- Coilfang Reservoir: Serpentshrine Cavern 2.1
-- =====================================================================================================

-- Hydross the Unstable (21216)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '650', `MinLevelHealth` = '4932200', `MaxLevelHealth` = '4932200' WHERE `entry` = '21216';
-- The Lurker Below (21217)+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '770', `MinLevelHealth` = '5842760', `MaxLevelHealth` = '5842760', `DamageMultiplier` = '42' WHERE `entry` = '21217';
-- Leotheras the Blind (21215)+ 30% (500H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '780', `MinLevelHealth` = '5918640', `MaxLevelHealth` = '5918640', `DamageMultiplier` = '28' WHERE `entry` = '21215';
-- Fathom-Lord Karathress (21214)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '390', `MinLevelHealth` = '2367300', `MaxLevelHealth` = '2367300', `DamageMultiplier` = '36' WHERE `entry` = '21214';
-- Fathom-Guard Caribdis (21964)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '195', `MinLevelHealth` = '1120080', `MaxLevelHealth` = '1120080', `DamageMultiplier` = '25' WHERE `entry` = '21964';
-- Fathom-Guard Tidalvess (21965)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '195', `MinLevelHealth` = '1120080', `MaxLevelHealth` = '1120080', `DamageMultiplier` = '42' WHERE `entry` = '21965';
-- Fathom-Guard Sharkkis (21966)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '195', `MinLevelHealth` = '1120080', `MaxLevelHealth` = '1120080', `DamageMultiplier` = '25' WHERE `entry` = '21966';
-- Morogrim Tidewalker (21213)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '975', `MinLevelHealth` = '7398300', `MaxLevelHealth` = '7398300', `DamageMultiplier` = '56' WHERE `entry` = '21213';
-- Tidewalker Lurker (21920)+ (10H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '35905', `MaxLevelHealth` = '35905' WHERE `entry` = '21920';
-- Lady Vashj (21212)+ 5%
UPDATE `creature_template` SET `HealthMultiplier` = '1050', `MinLevelHealth` = '6373500', `MaxLevelHealth` = '6373500', `DamageMultiplier` = '45' WHERE `entry` = '21212';
-- Coilfang Elite (22055)+
UPDATE `creature_template` SET `HealthMultiplier` = '30', `MinLevelHealth` = '209580', `MaxLevelHealth` = '209580', `DamageMultiplier` = '32' WHERE `entry` = '22055';
-- Toxic Spore Bat (22140)+ (41916)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986' WHERE `entry` = '22140';
-- Coilfang Strider (22056)+
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '349300', `MaxLevelHealth` = '349300', `DamageMultiplier` = '60' WHERE `entry` = '22056';

-- ============================
-- Coilfang Reservoir: Serpentshrine Cavern Trash 2.1
-- ============================

-- untested nerf, might be too much
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 0.8, `MinLevelHealth` = `MinLevelHealth` * 0.8, `MaxLevelHealth` = `MaxLevelHealth` * 0.8 WHERE `entry` IN (
21251, -- Underbog Colossus (21251) (75H/30D)
22352, -- Colossus Rager 22352 (4H/3D)
21221, -- Coilfang Beast-Tamer (21221) (35H/28D)
21224, -- Tidewalker Depth-Seer 21224 (17.5H/20D)
21225, -- Tidewalker Warrior 21225 (17.5H/20D)
21226, -- Tidewalker Shaman 21226 (17.5H/16D)
21227, -- Tidewalker Harpooner 21227 (17.5H/20D)
21228, -- Tidewalker Hydromancer 21228 (17.5H/16D)
21229, -- Greyheart Tidecaller 21229 (25H/20D)
21298, -- Coilfang Serpentguard (21298) (25H/26D)
21299, -- Coilfang Fathom-Witch 21299 (25H/16D)
21230, -- Greyheart Nether-Mage 21230 (25H/16D)
21231, -- Greyheart Shield-Bearer 21231 (25H/25D)
21232, -- Greyheart Skulker 21232 (25H/35D)
21339, -- Coilfang Hate-Screamer 21339 (25H/16D)
21863, -- Serpentshrine Lurker 21863 (10.5H/24D)
21263, -- Greyheart Technician 21263 (2H/4D)
21220, -- Coilfang Priestess (21220) (25H/16D)
21301, -- Coilfang Shatterer (21301) (25H/26D)
21218, -- Vashj'ir Honor Guard (21218) (24.5H/25D)
21246, -- Serpentshrine Sporebat 21246 (10.5H/15D)
22347, -- Colossus Lurker 22347 (20H/25D)
22238 -- Serpentshrine Tidecaller 22238 (15H/18D)
);

-- =====================================================================================================
-- Tempest Keep: The Eye 2.1
-- =====================================================================================================

-- Al'ar 19514+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '495', `MinLevelHealth` = '3756060', `MaxLevelHealth` = '3756060', `DamageMultiplier` = '30' WHERE `entry` = '19514';
-- Void Reaver 19516+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '975', `MinLevelHealth` = '5918250', `MaxLevelHealth` = '5918250', `DamageMultiplier` = '36' WHERE `entry` = '19516';
-- High Astromancer Solarian 18805+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '975', `MinLevelHealth` = '5918250', `MaxLevelHealth` = '5918250', `DamageMultiplier` = '25' WHERE `entry` = '18805';
-- Kael'thas Sunstrider 19622+ 5% (40D)
UPDATE `creature_template` SET `HealthMultiplier` = '1050', `MinLevelHealth` = '6373500', `MaxLevelHealth` = '6373500', `DamageMultiplier` = '48' WHERE `entry` = '19622';
-- Lord Sanguinar 20060, Grand Astromancer Capernian 20062, Master Engineer Telonicus 20063, Thaladred the Darkener 20064
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` / 2, `MinLevelHealth` = `MinLevelHealth` / 2, `MaxLevelHealth` = `MaxLevelHealth` / 2 WHERE `entry` IN (20060,20062,20063,20064);
-- Phoenix Egg 21364- 20%
UPDATE `creature_template` SET `HealthMultiplier` = '20', `MinLevelHealth` = '139720', `MaxLevelHealth` = '139720' WHERE `entry` = '21364';

-- ============================
-- Tempest Keep: The Eye Trash 2.1
-- ============================

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 0.8, `MinLevelHealth` = `MinLevelHealth` * 0.8, `MaxLevelHealth` = `MaxLevelHealth` * 0.8 WHERE `entry` IN (
20031, -- Bloodwarder Legionnaire 20031 (25H/28D)
20032, -- Bloodwarder Vindicator 20032 (40H/32D)
20033, -- Astromancer 20033 (25H/14D)
20034, -- Star Scryer 20034 (25H/16D)
20035, -- Bloodwarder Marshal 20035 (40H/32D)
20036, -- Bloodwarder Squire 20036 (25H/24D)
20037, -- Tempest Falconer 20037 (17.5H/10D)
20038, -- Phoenix-Hawk Hatchling 20038 (7H/14D)
20039, -- Phoenix-Hawk 20039 (10H/16D)
20040, -- Crystalcore Devastator 20040 (75H/32D)
20041, -- Crystalcore Sentinel 20041 (40H/28D)
20042, -- Tempest-Smith 20042 (17.5H/13D)
20043, -- Apprentice Star Scryer 20043 (2.8H/10D)
20044, -- Novice Astromancer 20044 (2.8H/10D)
20045, -- Nether Scryer 20045 (40H/20D)
20046, -- Astromancer Lord 20046 (40H/20D)
20047, -- Crimson Hand Battle Mage 20047 (17.5H/16D)
20048, -- Crimson Hand Centurion 20048 (17.5H/15D)
20049, -- Crimson Hand Blood Knight 20049 (28H/16D)
20050, -- Crimson Hand Inquisitor 20050 (28H/15D)
20052 -- Crystalcore Mechanic 20052 (25H/20D)
);

-- =====================================================================================================
-- Caverns of Time: Hyjal Summit 2.1
-- =====================================================================================================

-- Rage Winterchill (17767)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '980', `MinLevelHealth` = '5948600', `MaxLevelHealth` = '5948600', `DamageMultiplier` = '45' WHERE `entry` = '17767';
-- Anetheron (17808)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '980', `MinLevelHealth` = '5948600', `MaxLevelHealth` = '5948600', `DamageMultiplier` = '50' WHERE `entry` = '17808';
-- Kaz'rogal (17888)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '840', `MinLevelHealth` = '5098800', `MaxLevelHealth` = '5098800', `DamageMultiplier` = '50' WHERE `entry` = '17888';
-- Azgalor (17842)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '980', `MinLevelHealth` = '5948600', `MaxLevelHealth` = '5948600', `DamageMultiplier` = '85' WHERE `entry` = '17842';
-- Archimonde (17968)+ 60%
UPDATE `creature_template` SET `HealthMultiplier` = '1200', `MinLevelHealth` = '7284000', `MaxLevelHealth` = '7284000', `DamageMultiplier` = '110' WHERE `entry` = '17968';

-- ============================
-- Caverns of Time: Hyjal Summit Trash 2.1
-- ============================



-- =====================================================================================================
-- Black Temple 2.1
-- =====================================================================================================

-- High Warlord Naj'entus (22887)+ 60% (40D)
UPDATE `creature_template` SET `HealthMultiplier` = '800', `MinLevelHealth` = '6070400', `MaxLevelHealth` = '6070400', `DamageMultiplier` = '40' WHERE `entry` = '22887';
-- Supremus (22898)+ 60% (70D)
UPDATE `creature_template` SET `HealthMultiplier` = '960', `MinLevelHealth` = '7284480', `MaxLevelHealth` = '7284480', `DamageMultiplier` = '70' WHERE `entry` = '22898';
-- Shade of Akama (22841)+ 60% (94D)
UPDATE `creature_template` SET `HealthMultiplier` = '211.2', `MinLevelHealth` = '1602586', `MaxLevelHealth` = '1602586', `DamageMultiplier` = '94' WHERE `entry` = '22841';
-- Teron Gorefiend (22871) (577.5H/80D) + 20%
UPDATE `creature_template` SET `HealthMultiplier` = '990', `MinLevelHealth` = '6009300', `MaxLevelHealth` = '6009300', `DamageMultiplier` = '80' WHERE `entry` = '22871';
-- Gurtogg Bloodboil (22948)+ 30% (42D)
UPDATE `creature_template` SET `HealthMultiplier` = '975', `MinLevelHealth` = '7398300', `MaxLevelHealth` = '7398300', `DamageMultiplier` = '42' WHERE `entry` = '22948';
-- Essence of Suffering (23418) (210H/4D) + 10%
UPDATE `creature_template` SET `HealthMultiplier` = '330', `MinLevelHealth` = '2504040', `MaxLevelHealth` = '2504040', `DamageMultiplier` = '4' WHERE `entry` = '23418';
-- Essence of Desire (23419) (280H/32D) + 10%
UPDATE `creature_template` SET `HealthMultiplier` = '440', `MinLevelHealth` = '3338720', `MaxLevelHealth` = '3338720', `DamageMultiplier` = '32' WHERE `entry` = '23419';
-- Essence of Anger (23420) (280H/32D) + 10%
UPDATE `creature_template` SET `HealthMultiplier` = '440', `MinLevelHealth` = '3338720', `MaxLevelHealth` = '3338720', `DamageMultiplier` = '32' WHERE `entry` = '23420';
-- Mother Shahraz (22947)+ 40% (54D)
UPDATE `creature_template` SET `HealthMultiplier` = '1050', `MinLevelHealth` = '6373500', `MaxLevelHealth` = '6373500', `DamageMultiplier` = '54' WHERE `entry` = '22947';
-- The Illidari Council 23426 (700H - 4890200)
UPDATE `creature_template` SET `HealthMultiplier` = '1000', `MinLevelHealth` = '6986000', `MaxLevelHealth` = '6986000' WHERE `entry` = '23426'; -- 6449500 (Bestiary 2.3)
-- Illidan Stormrage (22917)+ 50% (94D)
UPDATE `creature_template` SET `HealthMultiplier` = '1200', `MinLevelHealth` = '9105600', `MaxLevelHealth` = '9105600', `DamageMultiplier` = '103.4' WHERE `entry` = '22917';
-- Flame of Azzinoth (22997)+ 50% (38D)
UPDATE `creature_template` SET `HealthMultiplier` = '225', `MinLevelHealth` = '1707300', `MaxLevelHealth` = '1707300', `DamageMultiplier` = '38' WHERE `entry` = '22997';

-- ============================
-- Black Temple Trash 2.1
-- ============================

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.3, `MinLevelHealth` = `MinLevelHealth` * 1.3, `MaxLevelHealth` = `MaxLevelHealth` * 1.3, `DamageMultiplier` = `DamageMultiplier` * 1.5 WHERE `entry` IN (
22855, -- Illidari Nightlord (22855) (60H/35D)
22878, -- Aqueous Lord (22878) (95H/26D)
22884, -- Leviathan (22884) (90H/43.5D)
22953, -- Wrathbone Flayer (22953) (50H/35D)
22954, -- Illidari Fearbringer (22954) (75H/30D)
22956, -- Sister of Pain (22956) (50H/30D)
22957, -- Priestess of Dementia (22957) (90H/26D)
22962, -- Priestess of Delight (22962) (90H/40D)
22964, -- Sister of Pleasure (22964) (50H/30D)
23049, -- Shadowmoon Weapon Master (23049) (50H/40D)
23172, -- Hand of Gorefiend (23172) (36H/30D)
23196, -- Bonechewer Behemoth (23196) (90H/30D)
23222, -- Bonechewer Brawler (23222) (55H/40D)
23239, -- Bonechewer Combatant (23239) (55H/40D)
23394, -- Promenade Sentinel (23394) (90H/32D)
23397, -- Illidari Blood Lord 23397 (40H/29D)
23400, -- Illidari Archon 23400 (30H/20D)
23402, -- Illidari Battle-Mage 23402 (30H/20D)
23403 -- Illidari Assassin 23403 (25H/25D)
);
UPDATE `creature_template` SET `DamageMultiplier` = '36' WHERE `entry` = '22878'; -- SPELL_SCHOOL_NATURE

-- =====================================================================================================
-- Zul'Aman 2.3
-- =====================================================================================================

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.40, `MinLevelHealth` = `MinLevelHealth` * 1.40, `MaxLevelHealth` = `MaxLevelHealth` * 1.40, `DamageMultiplier` = `DamageMultiplier` * 1.2 WHERE `entry` IN (
23574, -- Akil'zon (23574)
23576, -- Nalorakk (23576)
23578, -- Jan'alai (23578)
23577, -- Halazzi (23577)
24143, -- Spirit of the Lynx (24143)
24239, -- Hex Lord Malacrass (24239)
24240, -- Alyson Antille (24240)
24241, -- Thurg (24241)
24242, -- Slither (24242)
24243, -- Lord Raadan (24243)
24244, -- Gazakroth (24244)
24245, -- Fenstalker (24245)
24246, -- Darkheart (24246)
24247, -- Koragg (24247)
23863 -- Zul'jin (23863)
);

-- Corrupted Lightning Totem 24224+ 40%
UPDATE spell_template SET EffectBasePoints1=12599 WHERE id = 43302; -- 8999

-- ============================
-- Zul'Aman Trash 2.3
-- ============================

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.3, `MinLevelHealth` = `MinLevelHealth` * 1.3, `MaxLevelHealth` = `MaxLevelHealth` * 1.3 WHERE `entry` IN (
23542, -- Amani'shi Axe Thrower
23580, -- Amani'shi Warbringer
23581, -- Amani'shi Medicine Man
23582, -- Amani'shi Tribesman
23584, -- Amani Bear
23586, -- Amani'shi Scout
23587, -- Amani'shi Reinforcement
23596, -- Amani'shi Flame Caster
23597, -- Amani'shi Guardian
23774, -- Amani'shi Trainer
23889, -- Amani'shi Savage
24059, -- Amani'shi Beast Tamer
24065, -- Amani'shi Handler
24138, -- Tamed Amani Crocolisk
-- 24159, -- Amani Eagle
24179, -- Amani'shi Wind Walker
24180, -- Amani'shi Protector
24217, -- Amani Bear Mount
-- 24225, -- Amani'shi Warrior
24374, -- Amani'shi Berserker
24530, -- Amani Elder Lynx
24549 -- Amani'shi Tempest
);

-- 																=================================
-- 																===  2.1 HEROIC DUNGEON NERF  ===
-- 																=================================


UPDATE `creature_template` SET `DamageMultiplier` = `DamageMultiplier` - 1 WHERE `entry` IN (
-- Trash
20258,20255,20264,20256,20265,20301,20302,20299,20321,20322,20320,20323,20309,20316,20315,20298,20311,20310,20312,20313,20300,20693,
20697,20698,20701,20696,20692,20695,20699,20691,20694,20688,20686,21989,21990,21988,20640,20642,20638,20648,20641,20639,20646,20650,
20647,20649,20655,20645,20652,20644,20656,20660,20661,20538,20537,20527,20526,20530,20528,20529,20545,20547,20546,20534,20532,20533,
20525,22129,22399,20740,22164,20744,22172,20741,22165,20742,22166,20743,22168,22170,22171,19884,19892,19888,19891,19885,19886,19889,
19890,19887,19903,19904,21842,21843,21841,20164,20188,20191,20192,20181,20180,20173,20175,20187,20193,20190,20174,20620,20625,20628,
20621,20623,20626,20622,20624,20627,21914,21917,18053,18054,18049,18048,18052,18055,18058,18051,18050,18057,18620,18608,18619,18617,
18615,18618,18610,18611,18609,20593,20591,20582,20576,20590,20589,20594,20587,20579,20586,20583,20578,20575,20574,20588,20584,20577,
20580,21608,21607,21614,21591,21593,21586,21595,21594,21619,21613,21596,21597,21598,21621,21610,21611,22346,21549,21563,21564,21560,
21555,21561,21554,21565,21562,21543,21542,
-- Bosses
18436,18433,18621,18601,18607,20568,20993,20585,19893,19895,19894,20168,20183,20184,20629,20630,20633,20268,20266,20306,20303,20690,
20706,20636,20653,20657,20535,20521,20531,20738,21712,20745,20737,21543,21525,21526,21533,21537,21551,21558,21581,21559,21582,21626,
21590,21624,21599,21600,21601
);
-- Bosses
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` - 2 WHERE `entry` IN (
18432,18433,18434,18436,18601,18607,18621,19893,19894,19895,20165,20168,20169,20183,20184,20266,20267,20268,20303,20306,20318,20521,
20531,20535,20568,20596,20597,20629,20630,20633,20636,20637,20653,20657,20690,20706,20737,20738,20745,20993,21525,21526,21533,21536,
21537,21551,21558,21559,21581,21582,21590,21599,21624,21626,21712,22930,23035,26338,26339
);
-- Special Trash
UPDATE `creature_template` SET `DamageMultiplier` = '18.0' WHERE `entry` IN (18604,21645); -- Felguard Annihilator (22D), Felguard Brute (18D)


*/
--															=============================
--															=====  Pre 2.4.3 Tuning =====
--															=============================

-- =====================================================================================================
-- Karazhan 2.4
-- =====================================================================================================

-- Attumen the Huntsman (15550)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400', `DamageMultiplier` = '24' WHERE `entry` = '15550';
-- Midnight (16151)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400', `DamageMultiplier` = '19' WHERE `entry` = '16151';
-- Attumen the Huntsman (16152)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '50', `MinLevelHealth` = '379400', `MaxLevelHealth` = '379400', `DamageMultiplier` = '24' WHERE `entry` = '16152';
-- Moroes (15687)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '51', `MinLevelHealth` = '386988', `MaxLevelHealth` = '386988', `DamageMultiplier` = '17' WHERE `entry` = '15687';
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.2, `MinLevelHealth` = `MinLevelHealth` * 1.2, `MaxLevelHealth` = `MaxLevelHealth` * 1.2 WHERE `entry` IN (
19872, -- Lady Catriona Von'Indi (80482 hp)
19874, -- Baron Rafe Dreuger
19875, -- Baroness Dorothea Millstipe
17007, -- Lady Keira Berrybuck
19873, -- Lord Crispin Ference (100598 hp)
19876 -- Lord Robin Daris
);
-- Maiden of Virtue (16457)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '70', `MinLevelHealth` = '424900', `MaxLevelHealth` = '424900', `DamageMultiplier` = '38' WHERE `entry` = '16457';
-- Dorothee (17535)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '151750', `MaxLevelHealth` = '151750' WHERE `entry` = '17535';
-- Tito (17548)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '5', `MinLevelHealth` = '34930', `MaxLevelHealth` = '34930' WHERE `entry` = '17548';
-- Roar (17546)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700' WHERE `entry` = '17546';
-- Strawman (17543)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700' WHERE `entry` = '17543';
-- Tinhead (17547)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '15', `MinLevelHealth` = '110700', `MaxLevelHealth` = '110700' WHERE `entry` = '17547';
-- The Crone (18168)+ 10H
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '212450', `MaxLevelHealth` = '212450' WHERE `entry` = '18168';
-- Romulo (17533)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '265580', `MaxLevelHealth` = '265580', `DamageMultiplier` = '22' WHERE `entry` = '17533';
-- Julianne (17534)+ 40%
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '212450', `MaxLevelHealth` = '212450', `DamageMultiplier` = '20' WHERE `entry` = '17534';
-- The Big Bad Wolf (17521)+ 100% (24D)
UPDATE `creature_template` SET `HealthMultiplier` = '100', `MinLevelHealth` = '758800', `MaxLevelHealth` = '758800', `DamageMultiplier` = '24' WHERE `entry` = '17521';
-- The Curator (15691)+ 0% (22D)
UPDATE `creature_template` SET `HealthMultiplier` = '189', `MinLevelHealth` = '1147230', `MaxLevelHealth` = '1147230', `DamageMultiplier` = '26' WHERE `entry` = '15691';
-- Terestian Illhoof (15688)+ 15%
UPDATE `creature_template` SET `HealthMultiplier` = '115', `MinLevelHealth` = '803390', `MaxLevelHealth` = '803390', `DamageMultiplier` = '26' WHERE `entry` = '15688';
-- Kil'rek (17229)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '12', `MinLevelHealth` = '58680', `MaxLevelHealth` = '58680', `DamageMultiplier` = '26' WHERE `entry` = '17229';
-- Fiendish Imp (17267)- 22%
UPDATE `creature_template` SET `HealthMultiplier` = '0.9', `MinLevelHealth` = '4260', `MaxLevelHealth` = '4401', `DamageMultiplier` = '2' WHERE `entry` = '17267';
-- Shade of Aran (16524)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '192', `MinLevelHealth` = '1019712', `MaxLevelHealth` = '1019712', `DamageMultiplier` = '15' WHERE `entry` = '16524';
-- Conjured Elemental (17167)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '4.5', `MinLevelHealth` = '31437', `MaxLevelHealth` = '31437', `DamageMultiplier` = '7' WHERE `entry` = '17167';
-- Prince Malchezaar (15690)+ 30% (43D)
UPDATE `creature_template` SET `HealthMultiplier` = '195', `MinLevelHealth` = '1479660', `MaxLevelHealth` = '1479660', `DamageMultiplier` = '35' WHERE `entry` = '15690';
-- Prince Malchezaar's Axes (17650)- (3D)
UPDATE `creature_template` SET `DamageMultiplier` = '2.0' WHERE `entry` = '17650';
-- Netherspite (15689)+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '220.0', `MinLevelHealth` = '1229580', `MaxLevelHealth` = '1229580', `DamageMultiplier` = '29' WHERE `entry` = '15689';
-- Nightbane (17225)+ 30%
UPDATE `creature_template` SET `HealthMultiplier` = '227.5', `MinLevelHealth` = '1726270', `MaxLevelHealth` = '1726270', `DamageMultiplier` = '38' WHERE `entry` = '17225';
-- Restless Skeleton (17261)+ 0% (2.5H/7D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `MinLevelHealth` = '16355', `MaxLevelHealth` = '17465', `DamageMultiplier` = '7' WHERE `entry` = '17261';
-- Hyakiss the Lurker (16179)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '265580', `MaxLevelHealth` = '265580', `DamageMultiplier` = '20' WHERE `entry` = '16179';
-- Shadikith the Glider 16180+ (35H/24D)
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '265580', `MaxLevelHealth` = '265580', `DamageMultiplier` = '29' WHERE `entry` = '16180';
-- Rokad the Ravager 16181+ (30H/34D)
UPDATE `creature_template` SET `HealthMultiplier` = '30.0', `MinLevelHealth` = '227640', `MaxLevelHealth` = '227640', `DamageMultiplier` = '40' WHERE `entry` = '16181';

-- ============================
-- Karazhan Trash 2.4
-- ============================

-- 1.00 to 1.2, rather on the lower end "-> befor value"
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.2, `MinLevelHealth` = `MinLevelHealth` * 1.2, `MaxLevelHealth` = `MaxLevelHealth` * 1.2 WHERE `entry` IN (
15547, -- Spectral Charger (9H)			-> 11H
15548, -- Spectral Stallion (9H)
15551, -- Spectral Stable Hand (6H)		-> 8.5H
-- 16170, -- Coldmist Stalker (3H)
-- 16171, -- Coldmist Widow (7H)
-- 16173, -- Shadowbat (3H)
-- 16174, -- Greater Shadowbat (8H)
-- 16175, -- Vampiric Shadowbat (6H)
-- 16176, -- Shadowbeast (6H)
-- 16177, -- Dreadbeast (7H)
-- 16178, -- Phase Hound (4H)
16389, -- Spectral Apprentice (6H)		-> 9H
16406, -- Phantom Attendant (6H)
16407, -- Spectral Servant (6H)			-> 8.5H
16408, -- Phantom Valet (9H)			-> 10H
-- 16409, -- Phantom Guest (2H)
16410, -- Spectral Retainer (8H)		-> 11H
-- 16411, -- Spectral Chef (6H)
-- 16412, -- Ghostly Baker (6H)
-- 16414, -- Ghostly Steward (8H)
16415, -- Skeletal Waiter (6H)			-> 7H
16424, -- Spectral Sentry (6H)			-> 9H
16425, -- Phantom Guardsman (6H)		-> 7H
-- 17067, -- Phantom Hound (2H)
16459, -- Wanton Hostess (8H)			-> 11H
16460, -- Night Mistress (8H)			-> 11H
16461, -- Concubine (8H)				-> 11H
-- 16468, -- Spectral Patron (2H)
16470, -- Ghostly Philanthropist (8H)	-> 9H
16471, -- Skeletal Usher (12H)			-> 16H
16472, -- Phantom Stagehand (9H)		-> 9.3H
16473, -- Spectral Performer (8H)		-> 11H
16481, -- Ghastly Haunt (10H)			-> 13H
16482, -- Trapped Soul (10H)
16485, -- Arcane Watchman (12H)			-> 16H
-- 16488, -- Arcane Anomaly (0.2H) -> Powermultiplier for tuning
16489, -- Chaotic Sentience (8H)		-> 10H
16491, -- Mana Feeder (2H)				-> 3H
16492, -- Syphoner (2H)					-> 3H
16504, -- Arcane Protector (16H)		-> 21H
16525, -- Spell Shade (6H)				-> 9H
16526, -- Sorcerous Shade (8H)			-> 11H
16529, -- Magical Horror (8H)			-> 10H
16530, -- Mana Warp (6H)				-> 8H
16539, -- Homunculus (3H)				-> 5H
16540, -- Shadow Pillager (6H)			-> 8H
16544, -- Ethereal Thief (8H)			-> 10H
16545, -- Ethereal Spellfilcher (10H)	-> 13H
16595, -- Fleshbeast (10H)
16596 -- Greater Fleshbeast  (16H)		-> 19H
);

-- =====================================================================================================
-- Gruul's Lair 2.4
-- =====================================================================================================

-- High King Maulgar (18831)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '120.0', `MinLevelHealth` = '910560', `MaxLevelHealth` = '910560', `DamageMultiplier` = '40.0' WHERE `entry` = '18831';
-- Krosh Firehand (18832)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '60.0', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200', `DamageMultiplier` = '30.0' WHERE `entry` = '18832';
-- Olm the Summoner (18834)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '60.0', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200', `DamageMultiplier` = '32.0' WHERE `entry` = '18834';
-- Kiggler the Crazed (18835)+ 20% (28D)
UPDATE `creature_template` SET `HealthMultiplier` = '60.0', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200', `DamageMultiplier` = '50.0' WHERE `entry` = '18835';
-- Blindeye the Seer (18836)+ 20% (7D)
UPDATE `creature_template` SET `HealthMultiplier` = '60.0', `MinLevelHealth` = '364200', `MaxLevelHealth` = '364200', `DamageMultiplier` = '25.0' WHERE `entry` = '18836';
-- Wild Fel Stalker (18847)
UPDATE `creature_template` SET `HealthMultiplier` = '8.0', `MinLevelHealth` = '60704', `MaxLevelHealth` = '60704', `DamageMultiplier` = '12.0' WHERE `entry` = '18847';
-- Gruul the Dragonkiller (19044)+ 0% (26D)
UPDATE `creature_template` SET `HealthMultiplier` = '633', `MinLevelHealth` = '4803204', `MaxLevelHealth` = '4803204', `DamageMultiplier` = '26.0' WHERE `entry` = '19044';

-- ============================
-- Gruul's Lair Trash 2.4
-- ============================

-- Lair Brute (19389)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '48.0', `MinLevelHealth` = '354240', `MaxLevelHealth` = '354240', `DamageMultiplier` = '35' WHERE `entry` = '19389';
-- Gronn-Priest (21350)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '50.4', `MinLevelHealth` = '297511', `MaxLevelHealth` = '297511', `DamageMultiplier` = '30' WHERE `entry` = '21350';

-- =====================================================================================================
-- Magtheridon's Lair 2.4
-- =====================================================================================================

-- Magtheridon (17257)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '635.0', `MinLevelHealth` = '4818380', `MaxLevelHealth` = '4818380', `DamageMultiplier` = '50.0' WHERE `entry` = '17257';
-- Hellfire Channeler (17256)- 33% (40H/20D)
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '242800', `MaxLevelHealth` = '242800' WHERE `entry` = '17256';

-- ============================
-- Magtheridon's Lair Trash 2.4
-- ============================

-- Hellfire Warder (18829)- 33% (32H / 22D)
UPDATE `creature_template` SET `HealthMultiplier` = '32', `MinLevelHealth` = '194799', `MaxLevelHealth` = '194799', `DamageMultiplier` = '33.0' WHERE `entry` = '18829';

-- =====================================================================================================
-- World Bosses 2.4
-- =====================================================================================================

-- Doomwalker (17711)+ 15%
UPDATE `creature_template` SET `HealthMultiplier` = '391.0', `MinLevelHealth` = '2966908', `MaxLevelHealth` = '2966908', `DamageMultiplier` = '36.0' WHERE `entry` = '17711';
-- Doom Lord Kazzak (18728)+ 5%
UPDATE `creature_template` SET `HealthMultiplier` = '180.6', `MinLevelHealth` = '1370393', `MaxLevelHealth` = '1370393', `DamageMultiplier` = '90.0' WHERE `entry` = '18728';

-- =====================================================================================================
-- Coilfang Reservoir: Serpentshrine Cavern 2.4
-- =====================================================================================================

-- Hydross the Unstable (21216)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '600', `MinLevelHealth` = '4552800', `MaxLevelHealth` = '4552800' WHERE `entry` = '21216';
-- The Lurker Below (21217)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '5311600', `MaxLevelHealth` = '5311600', `DamageMultiplier` = '42' WHERE `entry` = '21217';
-- Leotheras the Blind (21215)+ 20% (500H/28D)
UPDATE `creature_template` SET `HealthMultiplier` = '720', `MinLevelHealth` = '5463360', `MaxLevelHealth` = '5463360', `DamageMultiplier` = '28' WHERE `entry` = '21215';
-- Fathom-Lord Karathress (21214)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '360', `MinLevelHealth` = '2185200', `MaxLevelHealth` = '2185200', `DamageMultiplier` = '36' WHERE `entry` = '21214';
-- Fathom-Guard Caribdis (21964)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '180', `MinLevelHealth` = '1033920', `MaxLevelHealth` = '1033920', `DamageMultiplier` = '25' WHERE `entry` = '21964';
-- Fathom-Guard Tidalvess (21965)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '180', `MinLevelHealth` = '1033920', `MaxLevelHealth` = '1033920', `DamageMultiplier` = '42' WHERE `entry` = '21965';
-- Fathom-Guard Sharkkis (21966)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '180', `MinLevelHealth` = '1033920', `MaxLevelHealth` = '1033920', `DamageMultiplier` = '25' WHERE `entry` = '21966';
-- Morogrim Tidewalker (21213)+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '900', `MinLevelHealth` = '6829200', `MaxLevelHealth` = '6829200', `DamageMultiplier` = '56' WHERE `entry` = '21213';
-- Tidewalker Lurker (21920)+ (10H/6D)
UPDATE `creature_template` SET `HealthMultiplier` = '2.5', `MinLevelHealth` = '17952.5', `MaxLevelHealth` = '17952.5' WHERE `entry` = '21920';
-- Lady Vashj (21212)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '1000', `MinLevelHealth` = '6070000', `MaxLevelHealth` = '6070000', `DamageMultiplier` = '45' WHERE `entry` = '21212';
-- Coilfang Elite (22055)+
UPDATE `creature_template` SET `HealthMultiplier` = '25', `MinLevelHealth` = '174650', `MaxLevelHealth` = '174650', `DamageMultiplier` = '31' WHERE `entry` = '22055';
-- Toxic Spore Bat (22140)+ (41916)
UPDATE `creature_template` SET `HealthMultiplier` = '1', `MinLevelHealth` = '6986', `MaxLevelHealth` = '6986' WHERE `entry` = '22140';
-- Coilfang Strider (22056)+
UPDATE `creature_template` SET `HealthMultiplier` = '40', `MinLevelHealth` = '279440', `MaxLevelHealth` = '279440', `DamageMultiplier` = '60' WHERE `entry` = '22056';

-- ============================
-- Coilfang Reservoir: Serpentshrine Cavern Trash 2.4
-- ============================

-- untested nerf, might be too much
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 0.75, `MinLevelHealth` = `MinLevelHealth` * 0.75, `MaxLevelHealth` = `MaxLevelHealth` * 0.75 WHERE `entry` IN (
21251, -- Underbog Colossus (21251) (75H/30D)
22352, -- Colossus Rager 22352 (4H/3D)
21221, -- Coilfang Beast-Tamer (21221) (35H/28D)
21224, -- Tidewalker Depth-Seer 21224 (17.5H/20D)
21225, -- Tidewalker Warrior 21225 (17.5H/20D)
21226, -- Tidewalker Shaman 21226 (17.5H/16D)
21227, -- Tidewalker Harpooner 21227 (17.5H/20D)
21228, -- Tidewalker Hydromancer 21228 (17.5H/16D)
21229, -- Greyheart Tidecaller 21229 (25H/20D)
21298, -- Coilfang Serpentguard (21298) (25H/26D)
21299, -- Coilfang Fathom-Witch 21299 (25H/16D)
21230, -- Greyheart Nether-Mage 21230 (25H/16D)
21231, -- Greyheart Shield-Bearer 21231 (25H/25D)
21232, -- Greyheart Skulker 21232 (25H/35D)
21339, -- Coilfang Hate-Screamer 21339 (25H/16D)
21863, -- Serpentshrine Lurker 21863 (10.5H/24D)
21263, -- Greyheart Technician 21263 (2H/4D)
21220, -- Coilfang Priestess (21220) (25H/16D)
21301, -- Coilfang Shatterer (21301) (25H/26D)
21218, -- Vashj'ir Honor Guard (21218) (24.5H/25D)
21246, -- Serpentshrine Sporebat 21246 (10.5H/15D)
22347, -- Colossus Lurker 22347 (20H/25D)
22238 -- Serpentshrine Tidecaller 22238 (15H/18D)
);

-- =====================================================================================================
-- Tempest Keep: The Eye 2.4
-- =====================================================================================================

-- Al'ar 19514+ 10%
UPDATE `creature_template` SET `HealthMultiplier` = '495', `MinLevelHealth` = '3756060', `MaxLevelHealth` = '3756060', `DamageMultiplier` = '30' WHERE `entry` = '19514';
-- Void Reaver 19516+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '900', `MinLevelHealth` = '5463000', `MaxLevelHealth` = '5463000', `DamageMultiplier` = '36' WHERE `entry` = '19516';
-- High Astromancer Solarian 18805+ 20%
UPDATE `creature_template` SET `HealthMultiplier` = '900', `MinLevelHealth` = '5463000', `MaxLevelHealth` = '5463000', `DamageMultiplier` = '25' WHERE `entry` = '18805';
-- Kael'thas Sunstrider 19622+ 0% (40D)
UPDATE `creature_template` SET `HealthMultiplier` = '1000', `MinLevelHealth` = '6070000', `MaxLevelHealth` = '6070000', `DamageMultiplier` = '40' WHERE `entry` = '19622';
-- Lord Sanguinar 20060, Grand Astromancer Capernian 20062, Master Engineer Telonicus 20063, Thaladred the Darkener 20064
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` / 2, `MinLevelHealth` = `MinLevelHealth` / 2, `MaxLevelHealth` = `MaxLevelHealth` / 2 WHERE `entry` IN (20060,20062,20063,20064);
-- Phoenix Egg 21364- 50%
UPDATE `creature_template` SET `HealthMultiplier` = '12.5', `MinLevelHealth` = '87325', `MaxLevelHealth` = '87325' WHERE `entry` = '21364';

-- ============================
-- Tempest Keep: The Eye Trash 2.4
-- ============================

-- untested nerf, might be too much
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 0.75, `MinLevelHealth` = `MinLevelHealth` * 0.75, `MaxLevelHealth` = `MaxLevelHealth` * 0.75 WHERE `entry` IN (
20031, -- Bloodwarder Legionnaire 20031 (25H/28D)
20032, -- Bloodwarder Vindicator 20032 (40H/32D)
20033, -- Astromancer 20033 (25H/14D)
20034, -- Star Scryer 20034 (25H/16D)
20035, -- Bloodwarder Marshal 20035 (40H/32D)
20036, -- Bloodwarder Squire 20036 (25H/24D)
20037, -- Tempest Falconer 20037 (17.5H/10D)
20038, -- Phoenix-Hawk Hatchling 20038 (7H/14D)
20039, -- Phoenix-Hawk 20039 (10H/16D)
20040, -- Crystalcore Devastator 20040 (75H/32D)
20041, -- Crystalcore Sentinel 20041 (40H/28D)
20042, -- Tempest-Smith 20042 (17.5H/13D)
20043, -- Apprentice Star Scryer 20043 (2.8H/10D)
20044, -- Novice Astromancer 20044 (2.8H/10D)
20045, -- Nether Scryer 20045 (40H/20D)
20046, -- Astromancer Lord 20046 (40H/20D)
20047, -- Crimson Hand Battle Mage 20047 (17.5H/16D)
20048, -- Crimson Hand Centurion 20048 (17.5H/15D)
20049, -- Crimson Hand Blood Knight 20049 (28H/16D)
20050, -- Crimson Hand Inquisitor 20050 (28H/15D)
20052 -- Crystalcore Mechanic 20052 (25H/20D)
);

-- =====================================================================================================
-- Caverns of Time: Hyjal Summit 2.4
-- =====================================================================================================

-- Rage Winterchill (17767)+ 30%
-- UPDATE `creature_template` SET `HealthMultiplier` = '910', `MinLevelHealth` = '5523700', `MaxLevelHealth` = '5523700', `DamageMultiplier` = '45' WHERE `entry` = '17767';
-- Anetheron (17808)+ 30%
-- UPDATE `creature_template` SET `HealthMultiplier` = '910', `MinLevelHealth` = '5523700', `MaxLevelHealth` = '5523700', `DamageMultiplier` = '50' WHERE `entry` = '17808';
-- Kaz'rogal (17888)+ 10%
-- UPDATE `creature_template` SET `HealthMultiplier` = '770', `MinLevelHealth` = '4673900', `MaxLevelHealth` = '4673900', `DamageMultiplier` = '50' WHERE `entry` = '17888';
-- Azgalor (17842)+ 30%
-- UPDATE `creature_template` SET `HealthMultiplier` = '910', `MinLevelHealth` = '5523700', `MaxLevelHealth` = '5523700', `DamageMultiplier` = '85' WHERE `entry` = '17842';
-- Archimonde (17968)+ 50%
-- UPDATE `creature_template` SET `HealthMultiplier` = '1125', `MinLevelHealth` = '6828750', `MaxLevelHealth` = '6828750', `DamageMultiplier` = '110' WHERE `entry` = '17968';

-- ============================
-- Caverns of Time: Hyjal Summit Trash 2.4
-- ============================



-- =====================================================================================================
-- Black Temple 2.4
-- =====================================================================================================

-- High Warlord Naj'entus (22887)+ 40% (40D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '700', `MinLevelHealth` = '5311600', `MaxLevelHealth` = '5311600', `DamageMultiplier` = '40' WHERE `entry` = '22887';
-- Supremus (22898)+ 40% (70D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '840', `MinLevelHealth` = '6373920', `MaxLevelHealth` = '6373920', `DamageMultiplier` = '70' WHERE `entry` = '22898';
-- Shade of Akama (22841)+ 40% (94D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '184.8', `MinLevelHealth` = '1402262', `MaxLevelHealth` = '1402262', `DamageMultiplier` = '94' WHERE `entry` = '22841';
-- Teron Gorefiend (22871) (577.5H/80D)+10%
-- UPDATE `creature_template` SET `HealthMultiplier` = '907.5', `MinLevelHealth` = '5508525', `MaxLevelHealth` = '5508525', `DamageMultiplier` = '80' WHERE `entry` = '22871';
-- Gurtogg Bloodboil (22948)+ 20% (42D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '900', `MinLevelHealth` = '6829200', `MaxLevelHealth` = '6829200', `DamageMultiplier` = '42' WHERE `entry` = '22948';
-- Essence of Suffering (23418) (210H/4D) +0%
-- UPDATE `creature_template` SET `HealthMultiplier` = '300', `MinLevelHealth` = '2276400', `MaxLevelHealth` = '2276400', `DamageMultiplier` = '4' WHERE `entry` = '23418';
-- Essence of Desire (23419) (280H/32D) + 0%
-- UPDATE `creature_template` SET `HealthMultiplier` = '400', `MinLevelHealth` = '3035200', `MaxLevelHealth` = '3035200', `DamageMultiplier` = '32' WHERE `entry` = '23419';
-- Essence of Anger (23420) (280H/32D) + 0%
-- UPDATE `creature_template` SET `HealthMultiplier` = '400', `MinLevelHealth` = '3035200', `MaxLevelHealth` = '3035200', `DamageMultiplier` = '32' WHERE `entry` = '23420';
-- Mother Shahraz (22947)+ 30% (54D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '975', `MinLevelHealth` = '5918250', `MaxLevelHealth` = '5918250', `DamageMultiplier` = '54' WHERE `entry` = '22947';
-- The Illidari Council 23426 (700H - 4890200)
UPDATE `creature_template` SET `HealthMultiplier` = '923', `MinLevelHealth` = '6448078', `MaxLevelHealth` = '6448078' WHERE `entry` = '23426'; -- 6449500 (Bestiary 2.3)
-- Illidan Stormrage (22917)+ 40% (94D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '1040', `MinLevelHealth` = '8498560', `MaxLevelHealth` = '8498560', `DamageMultiplier` = '94' WHERE `entry` = '22917';
-- Flame of Azzinoth (22997)+ 40% (38D)
-- UPDATE `creature_template` SET `HealthMultiplier` = '210', `MinLevelHealth` = '1593480', `MaxLevelHealth` = '1593480', `DamageMultiplier` = '38' WHERE `entry` = '22997';

-- ============================
-- Black Temple Trash 2.4
-- ============================

-- this might need a reduce in 2.3 (1.2) and 2.4 (1.0) to get smoother values and allow guilds to clear trash faster with newer content release in 2.4
-- UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.0, `MinLevelHealth` = `MinLevelHealth` * 1.0, `MaxLevelHealth` = `MaxLevelHealth` * 1.0, `DamageMultiplier` = `DamageMultiplier` * 1.4 WHERE `entry` IN (
-- 22855, -- Illidari Nightlord (22855) (60H/35D)
-- 22878, -- Aqueous Lord (22878) (95H/26D)
-- 22884, -- Leviathan (22884) (90H/43.5D)
-- 22953, -- Wrathbone Flayer (22953) (50H/35D)
-- 22954, -- Illidari Fearbringer (22954) (75H/30D)
-- 22956, -- Sister of Pain (22956) (50H/30D)
-- 22957, -- Priestess of Dementia (22957) (90H/26D)
-- 22962, -- Priestess of Delight (22962) (90H/40D)
-- 22964, -- Sister of Pleasure (22964) (50H/30D)
-- 23049, -- Shadowmoon Weapon Master (23049) (50H/40D)
-- 23172, -- Hand of Gorefiend (23172) (36H/30D)
-- 23196, -- Bonechewer Behemoth (23196) (90H/30D)
-- 23222, -- Bonechewer Brawler (23222) (55H/40D)
-- 23239, -- Bonechewer Combatant (23239) (55H/40D)
-- 23394, -- Promenade Sentinel (23394) (90H/32D)
-- 23397, -- Illidari Blood Lord 23397 (40H/29D)
-- 23400, -- Illidari Archon 23400 (30H/20D)
-- 23402, -- Illidari Battle-Mage 23402 (30H/20D)
-- 23403 -- Illidari Assassin 23403 (25H/25D)
-- );
-- UPDATE `creature_template` SET `DamageMultiplier` = '31' WHERE `entry` = '22878'; -- SPELL_SCHOOL_NATURE

-- =====================================================================================================
-- Zul'Aman 2.4
-- =====================================================================================================

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.30, `MinLevelHealth` = `MinLevelHealth` * 1.30, `MaxLevelHealth` = `MaxLevelHealth` * 1.30 WHERE `entry` IN (
23574, -- Akil'zon (23574)
23576, -- Nalorakk (23576)
23578, -- Jan'alai (23578)
23577, -- Halazzi (23577)
24143, -- Spirit of the Lynx (24143)
24239, -- Hex Lord Malacrass (24239)
24240, -- Alyson Antille (24240)
24241, -- Thurg (24241)
24242, -- Slither (24242)
24243, -- Lord Raadan (24243)
24244, -- Gazakroth (24244)
24245, -- Fenstalker (24245)
24246, -- Darkheart (24246)
24247, -- Koragg (24247)
23863 -- Zul'jin (23863)
);

-- Corrupted Lightning Totem 24224+ 30%
UPDATE spell_template SET EffectBasePoints1=11699 WHERE id = 43302; -- 8999

-- ============================
-- Zul'Aman Trash 2.4
-- ============================

UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.25, `MinLevelHealth` = `MinLevelHealth` * 1.25, `MaxLevelHealth` = `MaxLevelHealth` * 1.25 WHERE `entry` IN (
23542, -- Amani'shi Axe Thrower
23580, -- Amani'shi Warbringer
23581, -- Amani'shi Medicine Man
23582, -- Amani'shi Tribesman
23584, -- Amani Bear
23586, -- Amani'shi Scout
23587, -- Amani'shi Reinforcement
23596, -- Amani'shi Flame Caster
23597, -- Amani'shi Guardian
23774, -- Amani'shi Trainer
23889, -- Amani'shi Savage
24059, -- Amani'shi Beast Tamer
24065, -- Amani'shi Handler
24138, -- Tamed Amani Crocolisk
-- 24159, -- Amani Eagle
24179, -- Amani'shi Wind Walker
24180, -- Amani'shi Protector
24217, -- Amani Bear Mount
-- 24225, -- Amani'shi Warrior
24374, -- Amani'shi Berserker
24530, -- Amani Elder Lynx
24549 -- Amani'shi Tempest
);

-- =====================================================================================================
-- Sunwell Plateau 2.4
-- =====================================================================================================

-- Kalecgos (24850)+ 30%
-- UPDATE `creature_template` SET `HealthMultiplier` = '585', `MinLevelHealth` = '3550950', `MaxLevelHealth` = '3550950' WHERE `entry` = '24850';
-- Sathrovarr the Corruptor (24892)+ 30%
-- UPDATE `creature_template` SET `HealthMultiplier` = '604.5', `MinLevelHealth` = '3669315', `MaxLevelHealth` = '3669315' WHERE `entry` = '24892';
-- Kalecgos (24891)+ 30%
-- UPDATE `creature_template` SET `HealthMultiplier` = '214.5', `MinLevelHealth` = '1302015', `MaxLevelHealth` = '1302015' WHERE `entry` = '24891';
-- Brutallus (24882)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '1384', `MinLevelHealth` = '10501792', `MaxLevelHealth` = '10501792' WHERE `entry` = '24882';
-- Felmyst (25038)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '1154', `MinLevelHealth` = '7004780', `MaxLevelHealth` = '7004780' WHERE `entry` = '25038';
-- Lady Sacrolash (25165)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '475', `MinLevelHealth` = '2883250', `MaxLevelHealth` = '2883250' WHERE `entry` = '25165';
-- Grand Warlock Alythess (25166)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '475', `MinLevelHealth` = '2883250', `MaxLevelHealth` = '2883250' WHERE `entry` = '25166';
-- M'uru (25741)- 10% - 2.4.3
UPDATE `creature_template` SET `HealthMultiplier` = '360.0', `MinLevelHealth` = '2731680', `MaxLevelHealth` = '2731680' WHERE `entry` = '25741';
-- Entropius (25840)- 10% - 2.4.3
UPDATE `creature_template` SET `HealthMultiplier` = '270.0', `MinLevelHealth` = '2048760', `MaxLevelHealth` = '2048760' WHERE `entry` = '25840';
-- Kil'jaeden <The Deceiver> (25315)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '2200', `MinLevelHealth` = '13354000', `MaxLevelHealth` = '13354000' WHERE `entry` = '25315';
-- Hand of the Deceiver (25588)+ 0%
UPDATE `creature_template` SET `HealthMultiplier` = '35', `MinLevelHealth` = '206605', `MaxLevelHealth` = '206605' WHERE `entry` = '25588';

-- ========================
-- Sunwell Dead Server Nerf
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 0.75, `MinLevelHealth` = `MinLevelHealth` * 0.75, `MaxLevelHealth` = `MaxLevelHealth` * 0.75 WHERE `entry` IN (
24850, -- Kalecgos
24892, -- Sathrovarr the Corruptor
24882, -- Brutallus
25038, -- Felmyst
25165, -- Lady Sacrolash
25166, -- Grand Warlock Alythess
25741, -- M'uru
25840, -- Entropius
25798, -- Shadowsword Berserker
25799, -- Shadowsword Fury Mage
25772, -- Void Sentinel
25315, -- Kil'jaeden <The Deceiver>
25708, -- Sinister Reflection
25588, -- Hand of the Deceiver

-- =========================
-- Sunwell Plateau Trash 2.4
-- =========================
/*
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` * 1.2, `MinLevelHealth` = `MinLevelHealth` * 1.2, `MaxLevelHealth` = `MaxLevelHealth` * 1.2 WHERE `entry` IN (
*/
25363, -- Sunblade Cabalist
25367, -- Sunblade Arch Mage
25368, -- Sunblade Slayer
25369, -- Sunblade Vindicator
25370, -- Sunblade Dusk Priest
25371, -- Sunblade Dawn Priest
25372, -- Sunblade Scout
25373, -- Shadowsword Soulbinder
25483, -- Shadowsword Manafiend
25484, -- Shadowsword Assassin
25485, -- Shadowsword Deathbringer
25486, -- Shadowsword Vanquisher
25506, -- Shadowsword Lifeshaper
25507, -- Sunblade Protector
25508, -- Shadowsword Guardian
25509, -- Priestess of Torment
25591, -- Painbringer
25592, -- Doomfire Destroyer
25593, -- Apocalypse Guard
25595, -- Chaos Gazer
25597, -- Oblivion Mage
25599, -- Cataclysm Hound
25837, -- Shadowsword Commander
25851, -- Volatile Fiend
25860, -- Blazing Infernal
25864, -- Felguard Slayer
25867, -- Sunblade Dragonhawk
25948, -- Doomfire Shard
26101 -- Fire Fiend
);

-- 																=================================
-- 																===  2.4 HEROIC DUNGEON NERF  ===
-- 																=================================

UPDATE `creature_template` SET `DamageMultiplier` = `DamageMultiplier` - 2 WHERE `entry` IN (
20258,20255,20264,20256,20265,20301,20302,20299,20321,20322,20320,20323,20309,20316,20315,20298,20311,20310,20312,20313,20300,20693,
20697,20698,20701,20696,20692,20695,20699,20691,20694,20688,20686,21989,21990,21988,20640,20642,20638,20648,20641,20639,20646,20650,
20647,20649,20655,20645,20652,20644,20656,20660,20661,20538,20537,20527,20526,20530,20528,20529,20545,20547,20546,20534,20532,20533,
20525,22129,22399,20740,22164,20744,22172,20741,22165,20742,22166,20743,22168,22170,22171,19884,19892,19888,19891,19885,19886,19889,
19890,19887,19903,19904,21842,21843,21841,20164,20188,20191,20192,20181,20180,20173,20175,20187,20193,20190,20174,20620,20625,20628,
20621,20623,20626,20622,20624,20627,21914,21917,18053,18054,18049,18048,18052,18055,18058,18051,18050,18057,18620,18608,18619,18617,
18615,18618,18610,18611,18609,20593,20591,20582,20576,20590,20589,20594,20587,20579,20586,20583,20578,20575,20574,20588,20584,20577,
20580,21608,21607,21614,21591,21593,21586,21595,21594,21619,21613,21596,21597,21598,21621,21610,21611,22346,21549,21563,21564,21560,
21555,21561,21554,21565,21562,21543,21542,
-- Bosses
18436,18433,18621,18601,18607,20568,20993,20585,19893,19895,19894,20168,20183,20184,20629,20630,20633,20268,20266,20306,20303,20690,
20706,20636,20653,20657,20535,20521,20531,20738,21712,20745,20737,21543,21525,21526,21533,21537,21551,21558,21581,21559,21582,21626,
21590,21624,21599,21600,21601
);
-- Bosses
UPDATE `creature_template` SET `HealthMultiplier` = `HealthMultiplier` - 4 WHERE `entry` IN (
18432,18433,18434,18436,18601,18607,18621,19893,19894,19895,20165,20168,20169,20183,20184,20266,20267,20268,20303,20306,20318,20521,
20531,20535,20568,20596,20597,20629,20630,20633,20636,20637,20653,20657,20690,20706,20737,20738,20745,20993,21525,21526,21533,21536,
21537,21551,21558,21559,21581,21582,21590,21599,21624,21626,21712,22930,23035,26338,26339
);
-- Special Trash
UPDATE `creature_template` SET `DamageMultiplier` = '14.0' WHERE `entry` IN (18604,21645); -- Felguard Annihilator (22D), Felguard Brute (18D)
/*
*/

