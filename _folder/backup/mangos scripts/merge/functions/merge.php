<?php
function merge($conn_merge, $conn_work) {
	global $mangos_merge;
	global $mangos_work;
	global $merge_file;
	global $user;
	global $password;
	global $realm_id;

	title('---------------------------------');
	title('---------- merge() --------------');
	title('---------------------------------');

	require_once(__DIR__ .'/../template.php');
	
    try {

	// drop database
	if ($conn_merge->exec("USE $mangos_merge") !== false) {
		$conn_merge->exec("DROP DATABASE $mangos_merge");
		info("Database $mangos_merge dropped");		
	}
	
	// create database
    $conn_merge->exec("CREATE DATABASE $mangos_merge");
    info("Database $mangos_merge created");	
	
	// upload $merge_file
	$command = "mysql -u$user -p$password $mangos_merge < $merge_file 2>&1";
	$output = exec($command, $outputArray, $returnValue);
	
	if ($returnValue !== 0) {
		abort($output);
	}
	
	success("$merge_file UPLOADED");

	// check uploaded data
	$check_tables = ['areatrigger_involvedrelation', 'creature_template', 'mangos_string', 'worldstate_name', 'pool_creature'];

	// Select the appropriate database
	$conn_merge->query("USE $mangos_merge");

	foreach ($check_tables as $table) {
		info("Checking table $table...");
		$stmt = $conn_merge->query("SHOW TABLES LIKE '$table'");
		$tableExists = $stmt->rowCount();

		if (!$tableExists) {
			throw new PDOException("Table $table does not exist");
		}
	}
	success("$merge_file CHECK PASSED");
	
	// update table structure
	$conn_merge->exec("alter table creature_template add column HealthMod float NOT NULL DEFAULT 0;");
	$conn_merge->exec("alter table creature_template add column DamageMod float NOT NULL DEFAULT 0;");
	success("UPLOADING TABLES STRUCRURE");
	
	// compare table structure
    // Сравнение структур таблиц
	foreach ($tables_to_compare as $table) {
		info("Checking table $table...");
		$stmt1 = $conn_merge->query("SHOW COLUMNS FROM `$table`");
		$columns1 = $stmt1->fetchAll(PDO::FETCH_ASSOC);

		$stmt2 = $conn_work->query("SHOW COLUMNS FROM `$table`");
		$columns2 = $stmt2->fetchAll(PDO::FETCH_ASSOC);

		$columnCount1 = count($columns1);
		$columnCount2 = count($columns2);

		if ($columnCount1 !== $columnCount2) {
			info("`$table` column count is different $columnCount1 vs $columnCount2");
			exit;
		}

		for ($i = 0; $i < $columnCount1; $i++) {
			$column1 = $columns1[$i];
			$column2 = $columns2[$i];

			if ($column1['Field'] !== $column2['Field'] || $column1['Type'] !== $column2['Type']) {
				info("`$table` structure is different on column ". $column1['Field'] . " vs " . $column2['Field']);
				exit;
			}
		}
	}	
	success("TABLES STRUCRURE ARE IDENTICAL");
	
	// we are fine, start merge
	
	success("RECREATE TABLES");
    foreach ($recreate_tables as $table) {
        $conn_merge->exec("DROP TABLE IF EXISTS $table");
		$conn_merge->exec("CREATE TABLE $mangos_merge.$table LIKE $mangos_work.$table");
		$conn_merge->exec("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table");
		
		$rowc = $conn_merge->prepare("SELECT COUNT(*) FROM $table");
		$rowc->execute();
		$rowCount = $rowc->fetchColumn();
		
		if ($table == 'fast_start_create_spell')
			continue;
		
		if ($rowCount <= 0)
			throw new PDOException("Table $table row count is $rowCount!");
		
		info("Table $table recreated with row count $rowCount");
	}
	
	success("REPLACE DATA");
    foreach ($replace_data_in_tables as $table) {
        $conn_merge->exec("TRUNCATE TABLE $table");
		$conn_merge->exec("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table");
		
		$rowc = $conn_merge->prepare("SELECT COUNT(*) FROM $table");
		$rowc->execute();
		$rowCount = $rowc->fetchColumn();
		
		if ($rowCount <= 0)
			throw new PDOException("Table $table row count is $rowCount!");
		
		info("Table $table data replaced with row count $rowCount");
	}

	success("PROCESSING TABLES");
	foreach ($custom_items_entries as $entry) {
		$stmt = $conn_work->query("SELECT 1 FROM item_template WHERE entry = $entry");
		$rows = $stmt->fetchAll();
		if (count($rows) != 1) throw new PDOException("item_template entry: $entry does not exist");
		
		$conn_merge->query("REPLACE INTO $mangos_merge.item_template SELECT * FROM $mangos_work.item_template WHERE entry = $entry");
		$conn_merge->query("REPLACE INTO $mangos_merge.item_loot_template SELECT * FROM $mangos_work.item_loot_template WHERE entry = $entry");	
	}

	// ------------------------------------------------
	// conditions
	if (isset($conditions_types)) {
		$table = 'conditions';
		foreach ($conditions_types as $type) {
			$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE type = $type");
			$stmt->execute();
			dieIfEmpty($stmt->rowCount(),$table); 	
		}		
	}
	
	// ------------------------------------------------
	// spell_template
	$table = 'spell_template';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE id >= $spell_template_start_id");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	// ------------------------------------------------
	// disenchant_loot_template
	if (isset($disenchant_loot_template_start_entry)) {
		$table = 'disenchant_loot_template';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $disenchant_loot_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}
	
	// ------------------------------------------------
	// npc_trainer
	if (isset($npc_trainer_start_entry)) {
		$table = 'npc_trainer';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $npc_trainer_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);		
	}
	
	// ------------------------------------------------
	// npc_vendor
	if (isset($npc_vendor_start_entry)) {
		$table = 'npc_vendor';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $npc_vendor_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}
	
	// ------------------------------------------------
	// creature
	$table = 'creature';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE guid >= $creature_start_guid");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	// ------------------------------------------------
	// creature_template
	$table = 'creature_template';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $creature_template_start_entry");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	// creature_questrelation
	if (isset($npc_vendor_start_entry)) {
		$table = 'creature_questrelation';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE id >= $creature_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}	

	// creature_involvedrelation
	if (isset($npc_vendor_start_entry)) {
		$table = 'creature_involvedrelation';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE id >= $creature_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}	

	// creature_loot_template
	if (isset($move_creature_loot)) {
		$table = 'creature_loot_template';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $creature_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}
	
	// ------------------------------------------------
	// gameobject
	if (isset($gameobject_start_guid)) {
		$table = 'gameobject';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE guid >= $gameobject_start_guid");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}

	// ------------------------------------------------
	// gameobject_template
	if (isset($gameobject_template_start_entry)) {
		$table = 'gameobject_template';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $gameobject_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}

	// ------------------------------------------------
	// quest_template
	if (isset($quest_start_entry)) {
		$table = 'quest_template';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $quest_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);		
	}
	
	// ------------------------------------------------
	// npc_text
	$table = 'npc_text';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE id >= $npc_text_start_id and id != 16777215");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	// ------------------------------------------------
	// game_event
	$table = 'game_event';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $game_event_start_entry");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	// game_event_time
	$table = 'game_event_time';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $game_event_start_entry");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	
	
	// ------------------------------------------------
	// reference_loot_template
	if (isset($reference_loot_template_start_entry)) {
		$table = 'reference_loot_template';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $reference_loot_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);	

		// reference_loot_template_names
		$table = 'reference_loot_template_names';
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $reference_loot_template_start_entry");
		$stmt->execute();
		dieIfEmpty($stmt->rowCount(),$table);
	}	

	// locales
	$table = 'gossip_texts';
	$stmt = $conn_merge->prepare("UPDATE $mangos_merge.$table a, $mangos_work.$table b SET a.content_loc8=b.content_loc8 where a.entry=b.entry");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	$table = 'mangos_string';
	$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE entry >= $mangos_string_start_entry");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	

	$table = 'script_texts';
	$stmt = $conn_merge->prepare("UPDATE $mangos_merge.$table a, $mangos_work.$table b SET a.content_loc8=b.content_loc8 where a.entry=b.entry");
	$stmt->execute();
	dieIfEmpty($stmt->rowCount(),$table);	
	
	success("PROCESSING LOCALES");
	foreach ($locales_recreate as $table) {
		$conn_merge->exec("DELETE FROM $table");
		$stmt = $conn_merge->prepare("INSERT INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table");
		$stmt->execute();
		// GENSENTODO delete +1
		dieIfEmpty($stmt->rowCount()+1,$table);			
	}		
	
	success("PROCESSING IGNORE STUFF");	
	// IGNORE just for sync
	$table = 'game_event_creature';
	$stmt = $conn_merge->prepare("INSERT IGNORE INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE event >= $game_event_start_entry");
	$stmt->execute();
	
	$table = 'game_event_gameobject';
	$stmt = $conn_merge->prepare("INSERT IGNORE INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE event >= $game_event_start_entry");
	$stmt->execute();

	$table = 'game_event_quest';
	$stmt = $conn_merge->prepare("INSERT IGNORE INTO $mangos_merge.$table SELECT * FROM $mangos_work.$table WHERE event >= $game_event_start_entry");
	$stmt->execute();

	success("MERGE IS DONE");		
    } catch (PDOException $e) {
        abort("Error: " . $e->getMessage());
    }
}