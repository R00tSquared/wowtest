-- shattrath npc position
update creature set position_x=-1844.47, position_y=5420.42, position_z=-12.4281, orientation=5.64549 where guid=96591;
update creature set position_x=-1842.82, position_y=5425.76, position_z=-12.4281, orientation=6.1183 where guid=96591;
update creature set position_x=-1866.53, position_y=5409.18, position_z=-12.4279, orientation=4.79571 where guid=96591;
update creature set position_x=-1926.24, position_y=5475.53, position_z=-12.3448, orientation=2.11185 where guid=70238;
update creature set position_x=-1843.08, position_y=5425.81, position_z=-12.4282, orientation=5.85205 where guid=96653;

-- boss souls for quest
insert into creature_loot_template select 19622,26563,100,0,1,1,0,0;
insert into creature_loot_template select 22917,26566,100,0,1,1,0,0;
insert into creature_loot_template select 21212,26561,100,0,1,1,0,0;
insert into creature_loot_template select 19044,26562,100,0,1,1,0,0;
insert into creature_loot_template select 25315,26565,100,0,1,1,0,0;
insert into creature_loot_template select 15690,26564,100,0,1,1,0,0;
insert into creature_loot_template select 17968,26567,100,0,1,1,0,0;
insert into creature_loot_template select 23863,26568,100,0,1,1,0,0;