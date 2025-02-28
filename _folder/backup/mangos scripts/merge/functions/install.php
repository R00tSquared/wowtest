<?php
function install($conn_install) {
	global $user_install;
	global $password;
	global $mangos_work;
	global $mangos_merge;
	global $realm_id;

	title('---------------------------------');
	title('---------- install() ------------');
	title('---------------------------------');
	
	$backup_dir = "/backup/merge";
	$work_backup = "$backup_dir/mangos".$realm_id."_tbc_".date("YmdHis").".sql";
	$merge_backup = "$backup_dir/mangos".$realm_id."_tbc_merge_".date("YmdHis").".sql";
	if (!is_dir($backup_dir))
		abort("$backup_dir is not directory");
	
	// dump work
	$command = "mysqldump -u$user_install -p$password $mangos_merge > $work_backup 2>&1";
	$output = exec($command, $outputArray, $returnValue);
	
	if ($returnValue !== 0) {
		abort($output);
	} else {
		info("$work_backup dumped");
	}
	
	// dump merge
	$command = "mysqldump -u$user_install -p$password $mangos_merge > $merge_backup 2>&1";
	$output = exec($command, $outputArray, $returnValue);
	
	if ($returnValue !== 0) {
		abort($output);
	} else {
		info("$merge_backup dumped");
	}

	// drop work
    $conn_install->exec("DROP DATABASE $mangos_work");
    info("Database $mangos_work dropped");

    $conn_install->exec("CREATE DATABASE $mangos_work");
    info("Database $mangos_work created");
	
	// upload work
	$command = "mysql -u$user_install -p$password $mangos_work < $merge_backup 2>&1";
	$output = exec($command, $outputArray, $returnValue);
	
	if ($returnValue !== 0) {
		abort($output);
	} else {
		info("$merge_backup uploaded");
	}
}