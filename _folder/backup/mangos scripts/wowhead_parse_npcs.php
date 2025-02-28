<?php
// CREATE TABLE `wowhead_npcs` (
  // `entry` int(11) DEFAULT NULL,
  // `lang` varchar(10) DEFAULT NULL,
  // `name` varchar(255) DEFAULT NULL,
  // `subname` varchar(255) DEFAULT NULL
// ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
require_once('mylib.php');
set_time_limit(0);

$links = [
'en' => 'https://www.wowhead.com/tbc/npc=',
'ru' => 'https://www.wowhead.com/tbc/ru/npc=',
];

$sql = $conn->q("select entry from mangos_tbcdef.creature_template where entry not in (select entry from script.wowhead_npcs)");

echo 'Start...'.PHP_EOL;

foreach($sql as $row) {
	$entry = $row['entry'];
	
	foreach($links as $lang => $link) {
		$info = [];
		
		while (!$html = file_get_contents($link.$entry)) {
			echo "Can't get file_get_contents... sleeping".PHP_EOL;
			sleep(5);
		}
		
		if (preg_match('/Возможно, его больше нет в игре/',$html) || preg_match('/doesn\'t exist. It may have been removed from the game./',$html)) {
			$conn->q("insert into script.wowhead_npcs select $entry,'$lang',null,null");
			continue;
		}
		
		preg_match('/\$.extend.*"name":"(.*?)",(.*?"tag":"(.*?)",)?/',$html,$out);

		
		$info['entry'] = $entry;
		$info['name'] = (isset($out[1])) ? stripslashes($out[1]) : null;
		$info['subname'] = (isset($out[3])) ? stripslashes($out[3]) : null;
		
		$conn->q("insert into script.wowhead_npcs (entry,lang,name,subname) values (?,?,?,?)",[
			$entry,
			$lang,
			$info['name'],
			$info['subname'],
		]);
		
		if ($conn->count !== 1)
			die("Die at $entry");		
	}
	
	if (empty($info))
		continue;


    echo $entry.' ok'.PHP_EOL;
}

// update creature_template a, script.wowhead_npcs b set a.name=b.name, a.subname=b.subname where a.entry=b.entry and a.entry not in (11058) and b.lang='en';
// update locales_creature a, script.wowhead_npcs b set a.name_loc8=b.name, a.subname_loc8=b.subname where a.entry=b.entry and b.lang='ru';