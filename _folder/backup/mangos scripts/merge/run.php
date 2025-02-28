<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

try { 
	require_once(__DIR__ .'/config.php');
	require_once(__DIR__ .'/functions/merge.php');
	require_once(__DIR__ .'/functions/check_locales.php');
	require_once(__DIR__ .'/functions/check_merge.php');
	require_once(__DIR__ .'/functions/apply_sql.php');
	require_once(__DIR__ .'/functions/common.php');
	require_once(__DIR__ .'/functions/install.php');

	$realm_id = isset($argv[1]) ? $argv[1] : null;
	$config = isset($argv[2]) ? $argv[2] : null;
	
	if (!$config || !file_exists($config))
		usage();
	
	require_once($config);
	
	switch ($realm_id) {
		case 3: $mangos_work = $mangos_work3; break;
		case 5: $mangos_work = $mangos_work5; break;
		default: usage();
	}

	$conn_merge = new PDO("mysql:host=localhost;dbname=$mangos_merge", $user, $password);
	$conn_work = new PDO("mysql:host=localhost;dbname=$mangos_work", $user, $password);
	$conn_install = new PDO("mysql:host=localhost;dbname=$mangos_work", $user_install, $password);

	if (!$conn_merge || !$conn_work || !$conn_install)
		throw new PDOException("Can't connect to database");

	$conn_merge->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	$conn_work->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	$conn_install->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	
	merge($conn_merge, $conn_work);
	check_merge($conn_merge, $conn_work);
	check_locales($conn_merge, $conn_work);
	apply_sql($conn_merge);
	install($conn_install);
	
	success("Everything is fine, enjoy merged database!");
	
} catch (PDOException $e) {
    info("[CRITICAL] " . $e->getMessage());
}
