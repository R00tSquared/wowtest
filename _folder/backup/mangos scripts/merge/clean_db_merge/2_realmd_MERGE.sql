-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! WATCH OUT!!! 
-- realmd_import

insert into votes select * from realmd_import.votes;
insert into account_change_email select * from realmd_import.account_change_email;

insert into account (id,username,email,joindate,expansion,registration_ip,coins,recruiter,premium_until,last_ip,last_local_ip,last_login) select 
account_id,
username,
email, 
join_date,
1,
registration_ip,
coins,
recruiter,
premium_until,
last_ip,
last_local_ip,
last_login
from realmd_import.account; 

insert into account_banned 
(type       ,
account_id ,
banned_at  ,
expires_at ,
banned_by  ,
unbanned_at,
unbanned_by,
reason     ,
active)
select punishment_type_id,account_id,punishment_date,expiration_date,punished_by,0,null,reason,active from realmd_import.account_punishment;

update account_banned set unbanned_at=expires_at where active=0;
update account_banned set type=1 where type=3;



insert into account_premium_codes select * from realmd_import.account_premium_codes;
insert into shop_log select * from realmd_import.shop_log;

insert into uptime select 3,starttime,startstring,uptime,maxplayers from characters3.uptime;
insert into uptime select 5,starttime,startstring,uptime,maxplayers from characters5.uptime;

insert into realmlist select realm_id,name,ip_address,port,icon,flags,timezone,1,0,8606 from realmd_import.realms;

-- update account a, realmd_import.account_permissions b set a.permission_mask

-- insert into account_gms select * from realmd_import.account_permissions where realm_id in (3,5) and permission_mask>1;
insert into account_gms select * from realmd_import.account_permissions where realm_id in (3,5) and permission_mask=131071;

insert into account_permaban select * from realmd_import.account_permaban;
insert into promo_codes select * from realmd_import.promo_codes;

insert into ip_blocks select * from mangos03.blocks;

insert into account_logons (accountId,ip,local_ip,loginTime,loginSource,LocId) select account_id,ip,local_ip,login_date,1,LocId from realmd_import.account_login;
