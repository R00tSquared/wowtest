<?php
function process_files($realm_id) {
	global $conn_merge;
	
	$dir = __DIR__ . "/../sql/$realm_id/";
	if (!is_dir($dir)) {
		die("$dir is null!");
	}

	$files = glob($dir . '*.sql');
	
	if (empty($files))
		die("$dir is empty dir");
	
	natsort($files);
	
	foreach ($files as $file) {
		if (empty(file_get_contents($file))) {
			die("Can't execute $file");
		}

		info("Executing $realm_id - $file...");

		if ($conn_merge->exec(file_get_contents($file)) === false) {
			die("Error executing SQL script: $file, rollback changes...");
		}
	}	
}

function apply_sql($conn_merge) {
	global $mangos_merge;
	global $realm_id;
	
	title('---------------------------------');
	title('--------- apply_sql() -----------');
	title('---------------------------------');
	
	$stmt = $conn_merge->prepare("SELECT `locked` FROM merge_lock");
	$stmt->execute();
	if ($stmt->fetchColumn() != 0) {
		die("merge_lock is locked");
	}

	$conn_merge->exec("UPDATE merge_lock SET locked=1");

	process_files($realm_id); //realm specified sql
	process_files(0); //sql for all realms

	success("All .sql files done");
	$conn_merge->exec("UPDATE merge_lock SET locked=0");
}