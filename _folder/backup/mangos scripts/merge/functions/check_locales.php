<?php
function check_locales($conn_merge, $conn_work) {
	global $mangos_merge;
	global $mangos_work;
	global $merge_file;
	global $user;
	global $password;
	
	title('---------------------------------');
	title('------- check_locales() ---------');
	title('---------------------------------');	
	
	// empty bu default, should be deleted?
	$table = 'custom_texts';
	$stmt = $conn_merge->prepare("select count(*) from $table");
	$stmt->execute();
	dieIfNotEmpty($stmt->fetchColumn(),$table);	

	// don't have locales_ table
	$table = 'gossip_texts';
	$column = 'content_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table where $column is null or $column=''");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);

	$table = 'script_texts';
	$column = 'content_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table where $column is null or $column=''");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);

	$table = 'broadcast_text';
	$column = 'Text_lang';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join broadcast_text_locale b on a.id=b.id where (b.$column is null or b.$column='') and a.text!='' and b.Locale = 'ruRU';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);

	// has locales_ table
	$table = 'trainer_greeting';
	$column = 'Text_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_trainer_greeting b on a.entry=b.entry where b.$column is null or b.$column='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	$table = 'questgiver_greeting';
	$column = 'Text_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_questgiver_greeting b on a.entry=b.entry where b.$column is null or b.$column='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);

	$table = 'points_of_interest';
	$column = 'icon_name_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_points_of_interest b on a.entry=b.entry where b.$column is null or b.$column='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	
	$table = 'page_text';
	$column = 'Text_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_page_text b on a.entry=b.entry where b.$column is null or b.$column='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	
	
	$npc_text_columns = [
		'Text0_0_loc8',
		'Text0_1_loc8',
		'Text1_0_loc8',
		'Text1_1_loc8',
		'Text2_0_loc8',
		'Text2_1_loc8',
		'Text3_0_loc8',
		'Text3_1_loc8',
		'Text4_0_loc8',
		'Text4_1_loc8',
		'Text5_0_loc8',
		'Text5_1_loc8',
		'Text6_0_loc8',
		'Text6_1_loc8',
		'Text7_0_loc8',
		'Text7_1_loc8'
	];
	
	foreach ($npc_text_columns as $column) {
		$table = 'npc_text';
		$def_column = str_replace('_loc8', '', $column);
		$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_npc_text b on a.id=b.entry where (b.$column is null or b.$column='') and a.$def_column != '';");
		$stmt->execute();
		echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	
	}
	
	$table = 'gossip_menu_option';
	$column = 'option_text_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_gossip_menu_option b on a.menu_id=b.menu_id and a.id=b.id where b.$column is null or b.$column = '';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	$table = 'gossip_menu_option';
	$column = 'box_text_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_gossip_menu_option b on a.menu_id=b.menu_id and a.id=b.id where (b.$column is null or b.$column = '') and a.box_text != '';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	$table = 'areatrigger_teleport';
	$column = 'Text_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_areatrigger_teleport b on a.id=b.entry where b.$column is null or b.$column='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);

	$quest_template_columns = [
		'Title_loc8',
		'Details_loc8',
		'Objectives_loc8',
		'OfferRewardText_loc8',
		'RequestItemsText_loc8',
		'EndText_loc8',
		'ObjectiveText1_loc8',
		'ObjectiveText2_loc8',
		'ObjectiveText3_loc8',
		'ObjectiveText4_loc8'
	];
	foreach ($quest_template_columns as $column) {
		$table = 'quest_template';
		$def_column = str_replace('_loc8', '', $column);
		$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_quest b on a.entry=b.entry where (b.$column is null or b.$column='') and a.$def_column!='';");
		$stmt->execute();
		echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	
	}
	

	$table = 'gameobject_template';
	$column = 'name_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_gameobject b on a.entry=b.entry where b.$column  is null or b.$column ='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	$table = 'gameobject_template';
	$column = 'castbarcaption_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_gameobject b on a.entry=b.entry where (b.$column  is null or b.$column ='') and a.castbarcaption!='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	
	
	
	$table = 'item_template';
	$column = 'name_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_item b on a.entry=b.entry where (b.$column is null or b.$column='') and a.entry in (select entry from $mangos_work.item_template);");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	$table = 'item_template';
	$column = 'description_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_item b on a.entry=b.entry where (b.$column is null or b.$column='') and a.description!='' and a.entry in (select entry from $mangos_work.item_template);");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	
	
	
	$table = 'item_template';
	$column = 'description_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_item b on a.entry=b.entry where (b.$column is null or b.$column='') and a.description!='' and a.entry in (select entry from $mangos_work.item_template);");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);
	
	$table = 'creature_template';
	$column = 'name_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_creature b on a.entry=b.entry where (b.$column is null or b.$column='');");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	

	$table = 'creature_template';
	$column = 'subname_loc8';
	$stmt = $conn_merge->prepare("select count(*) from $table a left join locales_creature b on a.entry=b.entry where (b.$column is null or b.$column='') and a.subname!='';");
	$stmt->execute();
	echoIfNotEmpty($stmt->fetchColumn(),$table,$column);	

	success("LOCALES CHECK IS DONE");	
}