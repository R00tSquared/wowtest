-- remove all Call to Arms creatures and gameobjects
delete from creature where guid in (select guid from game_event_creature where event in (18,19,20,21));
delete from creature_movement where id in (select guid from game_event_creature where event in (18,19,20,21));
delete from gameobject where guid in (select guid from game_event_gameobject where event in (18,19,20,21));
delete from game_event_creature where event in (18,19,20,21);
delete from game_event_gameobject where event in (18,19,20,21);