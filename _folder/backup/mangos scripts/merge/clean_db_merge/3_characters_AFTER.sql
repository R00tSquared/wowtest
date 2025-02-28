
-- x100
-- x100
-- x100
-- x100
-- x100
-- x100
-- x100
-- x100
-- x100
-- x100
-- x100
insert into mail_external select distinct null,guid,'Refund','Refund for Reins of Bronze Dragon',0,5564,38 from character_inventory where item_template=20221;
insert into mail_external select distinct null,guid,'Refund','Refund for Red Qiraji Resonating Crystal',0,1199,20 from character_inventory where item_template=27782;
insert into mail_external select distinct null,guid,'Refund','Refund for Rotten Bear',0,1199,20 from character_inventory where item_template=1691;

update item_instance set itemEntry=28482 where itemEntry=23943;
update character_inventory set item_template=28482 where item_template=23943;
update item_instance set itemEntry=10284 where itemEntry=18964;
update character_inventory set item_template=10284 where item_template=18964;

-- deprecated items
DELETE item_instance, character_inventory
FROM item_instance
JOIN character_inventory ON item_instance.guid = character_inventory.item
WHERE item_instance.itemEntry in (20020,18002,12866,6082,3034,22455,693152,693171,34077,10448,20484,31665,23775,26560,37598,23026,13215,27863,21717,21719,11663,8148,4559,5171,4452,20221,23943,18964,27782,5331,1136,23224);

-- fireworks, snowballs
DELETE item_instance, character_inventory
FROM item_instance
JOIN character_inventory ON item_instance.guid = character_inventory.item
WHERE item_instance.itemEntry in (9317,17202);



update item_instance set itemEntry=4618 where itemEntry=35463;
update character_inventory set item_template=4618 where item_template=35463;

INSERT INTO `shop_items` (`id`, `item`, `price`) VALUES (121, 4618, 24);

-- EXPECTED CLEAN CHARACTERS MOVED FROM LIVE
update characters set extra_flags=0;

update characters a, characters3_import.characters b set a.extra_flags = a.extra_flags | 8192 where a.guid=b.guid and b.char_custom_flags & 1;
update characters a, characters3_import.characters b set a.extra_flags = a.extra_flags | 16384 where a.guid=b.guid and b.char_custom_flags & 2;
update characters a, characters3_import.characters b set a.extra_flags = a.extra_flags | 32768 where a.guid=b.guid and b.char_custom_flags & 4;
update characters a, characters3_import.characters b set a.extra_flags = a.extra_flags | 8 where a.guid=b.guid and b.char_custom_flags & 8;

delete from guild_bank_item where item_entry=13215;
delete from character_action where spec>0;

INSERT INTO `world_state` (`Id`, `Data`) VALUES (20, '3 15 0 0 0 0 0 0 0 0 0 0 3 0 0 0');

-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
-- x5
update item_instance set itemEntry=33225 where itemEntry=5331;
update character_inventory set item_template=33225 where item_template=5331;

delete from item_instance where itemEntry in (18002,12866,6082,3034,22455,693152,693171,34077,10448,20484,31665,23775,26560,37598,23026,13215,27863,21717,21719,11663,8148,4559,5171,4452,20221,23943,18964,27782,5331,1136,23224);
delete from character_inventory where item_template in (18002,12866,6082,3034,22455,693152,693171,34077,10448,20484,31665,23775,26560,37598,23026,13215,27863,21717,21719,11663,8148,4559,5171,4452,20221,23943,18964,27782,5331,1136,23224);

-- EXPECTED CLEAN CHARACTERS MOVED FROM LIVE
update characters set extra_flags=0;

update characters a, characters5_import.characters b set a.extra_flags = a.extra_flags | 8192 where a.guid=b.guid and b.char_custom_flags & 1;
update characters a, characters5_import.characters b set a.extra_flags = a.extra_flags | 16384 where a.guid=b.guid and b.char_custom_flags & 2;
update characters a, characters5_import.characters b set a.extra_flags = a.extra_flags | 32768 where a.guid=b.guid and b.char_custom_flags & 4;
update characters a, characters5_import.characters b set a.extra_flags = a.extra_flags | 8 where a.guid=b.guid and b.char_custom_flags & 8;

delete from character_action where spec>0;