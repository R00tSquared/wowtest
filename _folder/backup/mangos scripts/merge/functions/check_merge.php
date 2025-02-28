<?php
// select a.entry,a.name,b.name from mangos3.item_template a left join item_template b on a.entry=b.entry where a.name!=b.name or b.entry is null;

// select a.entry,a.name from mangos_new.item_template a left join mangos3.item_template b on a.entry=b.entry where a.entry in (2363,23796,35463,33573,33572,33570,30471,2554,3513,21233,23656,23700,23701,1199,5564,734,1698,1699,20221,23943,20366,32372,38234,7388,7986,7987,7988,16086,16102,16103,16104,35519,35520,35521,30796,35517,23698,8962,8963,8954,8955,8958,8960,8961,8964,8965,8966,23086,5172,2767,23689,8967,8968,8969,20020,28877,4966,21065,23961,695005,695004,695003,695002,695000,693427,693425,693424,693423,693422,693421,693420,693419,693418,693417,693416,693415,693413,693412,693408,693407,693404,693403,693402,693400,693392,693391,693390,693389,693388,693387,693386,693385,693384,693383,693382,693381,693380,693285,693284,693283,693282,693281,693280,693279,693278,693277,693276,693275,693274,693273,693272,693271,693270,693269,693268,693267,693266,693265,693264,693263,693262,693261,693260,693259,693258,693257,693256,693255,693254,693253,693252,693251,693250,693249,693248,693247,693246,693245,693244,693243,693241,693240,693239,693238,693237,693236,693234,693233,693232,693231,693229,693228,693227,693226,693225,693222,693218,693217,693215,693214,693212,693210,693209,693208,693207,693206,693205,693204,693201,693182,693181,693180,693179,693177,693176,693175,693174,693172,693167,693166,693165,693164,693163,693162,693161,693149,693148,693147,693146,693145,693144,693143,693142,693141,693140,693139,693138,693137,693136,693135,693134,693133,693132,693131,693130,693129,693128,693127,693126,693125,693124,693123,693122,693121,693120,693119,693118,693117,693116,693115,693114,693113,693112,693107,693106,693105,693104,693103,693037,693036,693035,693003,693002,28171,875,1042,1084,1691,3580,5049,5384,5697,5698,5857,6208,6209,10478,10594,12440,19932,21141,21811,21879,22118,22124,22125,22126,22127,22129,23712,23861,23872,27318,27319,34519,7246,22475,23794,10719,23224) and b.entry is null;

function check_merge($conn_merge, $conn_work)
{
	global $mangos_merge;
	global $mangos_work;

	title('---------------------------------');
	title('-------- check merge() ----------');
	title('---------------------------------');
	
	$tables = array(
		'creature_template' => ['entry','name'],
		'gameobject_template' => ['entry','name'],
		'creature_loot_template' => ['entry','item'],
		'item_loot_template' => ['entry','item'],
		'gameobject_loot_template' => ['entry','item'],
		'reference_loot_template' => ['entry','item'],
		'mangos_string' => ['entry','content_default'],
		'npc_vendor' => ['entry','item'],
		'npc_trainer' => ['entry','spell'],
		'spell_template' => ['id','SpellName'],
		'npc_text' => ['id','text0_0'],
		'waypoint_path' => ['PathId','Comment'],
		'game_event' => ['entry','description'],
		'game_event_creature' => ['guid','event'],
		'game_event_gameobject' => ['guid','event'],
		'creature' => ['guid','id'],
		'gameobject' => ['guid','id'],
	);   
	
	$warnings = [];
	
	foreach ($tables as $table => $column) {
		$first = $column[0];
		$second = $column[1];
		
		try {
		$stmt = $conn_merge->prepare("SELECT a.$first,a.$second FROM $mangos_work.$table a LEFT JOIN $mangos_merge.$table b ON a.$first = b.$first WHERE b.$first IS NULL");
		$stmt->execute();

		$results = $stmt->fetchAll(PDO::FETCH_ASSOC);
		if (!empty($results)) {
			$msg = "Data in table '$table' exist in $mangos_work but not in $mangos_merge!";
			warning($msg);
			$warnings[] = $msg;
			echo "$first $second" . PHP_EOL;
			foreach ($results as $row) {
				info($row[$first]." ".$row[$second]);
			}
		}

		} catch (PDOException $e) {
            info("Error in table '$table': " . $e->getMessage());
        }
		
		//info("$table is fine...");
    }
	
	foreach ($warnings as $warn) {
		warning($warn);
	}
	
	success("CHECK IS DONE");	
}