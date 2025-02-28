-- update passwords

insert into mw_site.account_data select account_id,vote_mask from realmd_import.account where vote_mask>0;
