-- reagent stack count
update item_template set stackable=200 where entry in (17026,17056,28056,28061,30796,3776,5140,5565,6265,9186,17020,17028,17029,17030,17031,17032,17033,17057,17058,21177,21927,22054,22055,22147,22148);

-- antique loot
drop table if exists antiq;
create temporary table antiq select entry from item_template where entry in (8586,11364,12302,12303,12330,12351,12353,12354,13317,13328,13329,13582,13583,13584,15292,15293,20131,20132,20371,22114,22999,23705,23709,23713,23716,25535,30360,32566,32588,32616,32617,32622,33079,33219,33223,33225,34480,34492,34493,34499,35223,35226,35227,37719,38233,38301,38309,38310,38311,38312,38313,38314,38576,39656);
update item_template set flags = flags | 0x10000000, flags = flags | 0x20000000, maxcount=0, RequiredDisenchantSkill=0, DisenchantID=100, allowablerace=-1, requiredskill=0, requiredskillrank=0, bonding=3, stackable=1, spellcharges_1=0, spellcharges_2=0 where entry in (select entry from antiq);
update item_template set bonding=2 where entry in (select entry from antiq) and inventorytype>0;

-- shattrath reagent vendors
drop table if exists shattr_reg;
create temporary table shattr_reg select item from mangos3_tbc.npc_vendor where entry in (693077,693079,693080,693085,693081,693084,693076,693078,693082,693083);
update item_template a, mangos3_tbc.item_template b set a.BuyCount=b.BuyCount, a.BuyPrice=b.BuyPrice, a.SellPrice=b.SellPrice where a.entry=b.entry and a.entry in (select item from shattr_reg);
update npc_vendor a, item_template b set a.maxcount=b.buycount where a.incrtime>0 and a.maxcount<b.buycount and a.item=b.entry and a.item in (select item from shattr_reg) and a.entry<690000;
update npc_vendor_template a, item_template b set a.maxcount=b.buycount where a.incrtime>0 and a.maxcount<b.buycount and a.item=b.entry and a.item in (select item from shattr_reg);

-- crafted items bonding
update item_template set bonding=1 where entry in (34361,34362,34363,34366,34367,34370,34372,34374,34376,34378,34380,32568,32571,32574,32577,32580,32582,32584,32586,32389,32390,32391,32392,32393,32394,32395,32396,32397,32398,32399,32400,32401,32402,32403,32404,32420,30046,30044,30042,30040,30038,30036,30034,30032,33122);