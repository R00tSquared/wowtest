<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
require_once('mylib.php');
set_time_limit(0);

$target_db = 'mangos3_tbc';
$sql = $conn->q("select a.*,b.* from $target_db.quest_template a left join $target_db.locales_quest b on a.entry=b.entry where a.ObjectiveText1!='' or a.ObjectiveText2!='' or a.ObjectiveText3!='' or a.ObjectiveText4!=''");

$columns = [
	['ObjectiveText1','ObjectiveText1_loc8'],
	['ObjectiveText2','ObjectiveText2_loc8'],
	['ObjectiveText3','ObjectiveText3_loc8'],
	['ObjectiveText4','ObjectiveText4_loc8'],
];

foreach($sql as $manos_en) {
	$entry = $manos_en['entry'];

	$wh_sql = $conn->q("select * from script.wowhead_quests where entry=$entry and lang='en'");
	
	$keymap[$entry] = [];
	
	foreach($wh_sql as $wowhead_en) {
		foreach ($columns as $col) {
			if (empty($manos_en[$col[0]]))
				continue;
			
			foreach ($columns as $col1) {
				if ($wowhead_en[$col1[0]] == $manos_en[$col[0]]) {
					$obj = $conn->qv("select $col1[0] as obj from script.wowhead_quests where entry=$entry and lang='ru' limit 1");
					if (!$obj)
						die("wtf?");
					
					$conn->q("update $target_db.locales_quest set $col[0]_loc8 = ? where entry = $entry",[$obj]);		
				}	
			}
		}		
	}
	
	echo "------------- $entry ok!".PHP_EOL;
}
// select a.entry,a.ObjectiveText2,b.ObjectiveText2 from quest_template a, script.wowhead_quests b where a.entry=b.entry and a.ObjectiveText2!=b.ObjectiveText2 and a.ObjectiveText2!='' and b.lang='en';

// --- UPDATE + THIS SCRIPT = GOOD LOCALES
// update locales_quest a, script.wowhead_quests b set a.Title_loc8=b.Title where a.entry=b.entry and b.lang='ru' and a.Title_loc8!='' and a.Title_loc8 is not null;	
// update locales_quest a, script.wowhead_quests b set a.Details_loc8=b.Details where a.entry=b.entry and b.lang='ru' and a.Details_loc8!='' and a.Details_loc8 is not null;	
// update locales_quest a, script.wowhead_quests b set a.Objectives_loc8=b.Objectives where a.entry=b.entry and b.lang='ru' and a.Objectives_loc8!='' and a.Objectives_loc8 is not null;	
// update locales_quest a, script.wowhead_quests b set a.OfferRewardText_loc8=b.OfferRewardText where a.entry=b.entry and b.lang='ru' and a.OfferRewardText_loc8!='' and a.OfferRewardText_loc8 is not null;	
// update locales_quest a, script.wowhead_quests b set a.RequestItemsText_loc8=b.RequestItemsText where a.entry=b.entry and b.lang='ru' and a.RequestItemsText_loc8!='' and a.RequestItemsText_loc8 is not null;	
// update locales_quest a, script.wowhead_quests b set a.EndText_loc8=b.EndText where a.entry=b.entry and b.lang='ru' and a.EndText_loc8!='' and a.EndText_loc8 is not null;