-- ---------------------------------------------------------
-- Netherwing - Released in 2.1
-- Comment the entire file to release
-- ---------------------------------------------------------

-- ---------------------------------------------------------
-- Disable all reputation gains with Netherwing
-- ---------------------------------------------------------
DELETE FROM reputation_reward_rate WHERE faction IN(1015);
INSERT INTO reputation_reward_rate (faction, quest_rate, creature_rate, spell_rate) VALUES
(1015,1,0,0); -- First part of quest chain is available in patch 2.0 (up to http://www.wowhead.com/quest=10870), so allow quest rep only

-- -------------------------------------------------------------------------------------------------
-- Despawn all creatures of the Netherwing/Dragonmaw faction
-- -------------------------------------------------------------------------------------------------
UPDATE creature SET spawnmask=0 WHERE id IN(SELECT entry FROM creature_template WHERE Faction IN(1863,1865,1850));
-- Some stragglers which are in the daily hubs but don't have the normal factions
UPDATE creature SET spawnmask=0 WHERE id=22433; -- Ja'y Nosliw
UPDATE creature SET spawnmask=0 WHERE id=23140; -- Taskmaster Varkule Dragonbreath
UPDATE creature SET spawnmask=0 WHERE id=23427; -- Illidari Lord Balthas
UPDATE creature SET spawnmask=0 WHERE id=23145; -- Rumpus
UPDATE creature SET spawnmask=0 WHERE id=23345; -- Wing Commander Ichhman
UPDATE creature SET spawnmask=0 WHERE id=23346; -- Wing Commander Mulverick
UPDATE creature SET spawnmask=0 WHERE id=23489; -- Drake Dealer Hurlunk
UPDATE creature SET spawnmask=0 WHERE id=23340; -- Murg "Oldie" Muckjaw
UPDATE creature SET spawnmask=0 WHERE id=23342; -- Trope the Filth-Belcher
UPDATE creature SET spawnmask=0 WHERE id=23344; -- Corlok the Vet
UPDATE creature SET spawnmask=0 WHERE id=23348; -- Captain Skyshatter
UPDATE creature SET spawnmask=0 WHERE id=23291; -- Chief Overseer Mudlump

-- Trash that drops Netherwing Items
UPDATE creature SET spawnmask=0 WHERE id=23169; -- Nethermine Flayer
UPDATE creature SET spawnmask=0 WHERE id=23264; -- Overmine Flayer
UPDATE creature SET spawnmask=0 WHERE id=23267; -- Arvoar the Rapacious
UPDATE creature SET spawnmask=0 WHERE id=23269; -- Barash the Den Mother
UPDATE creature SET spawnmask=0 WHERE id=23285; -- Nethermine Burster
UPDATE creature SET spawnmask=0 WHERE id=23286; -- Black Blood of Draenor
UPDATE creature SET spawnmask=0 WHERE id=23290; -- Draenor Blood Terror
UPDATE creature SET spawnmask=0 WHERE id=23305; -- Crazed Murkblood Foreman
UPDATE creature SET spawnmask=0 WHERE id=23324; -- Crazed Murkblood Miner
UPDATE creature SET spawnmask=0 WHERE id=23326; -- Nethermine Ravager
UPDATE creature SET spawnmask=0 WHERE id=23501; -- Netherwing Ray

-- Netherwing
UPDATE quest_template SET minlevel = 80 WHERE entry IN (11012);
-- Disable Netherwing Drakes and Nether Rays
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (32314,32316,32317,32318,32319);
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (32857,32858,32859,32860,32861,32862);
-- Faction items for Netherwing
UPDATE item_template SET SellPrice=0, ItemLevel=1, RequiredLevel=80 WHERE entry IN (32863,32694,32695,32864);


