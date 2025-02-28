File example: 1678718967_35.sql
- 1678718967 -> your current unixtime (can be found with select UNIX_TIMESTAMP();)
- 35 -> changes must be applied for this realms (3 - x100, 5 - x5)

Files should not contain database specified queries like:
- "mangos5.creature_template" or "use mangos5; update creature_template..."