<?php
// CREATE TABLE `wowhead_quests` (
  // `entry` int(11) DEFAULT NULL,
  // `lang` varchar(10) DEFAULT NULL,
  // `Title` text DEFAULT NULL,
  // `Details` text DEFAULT NULL,
  // `Objectives` text DEFAULT NULL,
  // `OfferRewardText` text DEFAULT NULL,
  // `RequestItemsText` text DEFAULT NULL,
  // `EndText` text DEFAULT NULL,
  // `ObjectiveText1` text DEFAULT NULL,
  // `ObjectiveText2` text DEFAULT NULL,
  // `ObjectiveText3` text DEFAULT NULL,
  // `ObjectiveText4` text DEFAULT NULL
// ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

// TODO WARNING!
// TODO WARNING!
// TODO WARNING!

// check this ID 98 |3-6(Охотник)

// TODO WARNING!
// TODO WARNING!
// TODO WARNING!

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
require_once('mylib.php');
set_time_limit(0);

require_once(__DIR__ . '/simple_html_dom.php');

$testing = 0;
$entries = [];

$links = [
	'en' => 'https://www.wowhead.com/tbc/quest=',
	'ru' => 'https://www.wowhead.com/tbc/ru/quest=',
];

function convert($str) {
	global $conn;
	global $testing;

	// html stuff
	$str = preg_replace('/<br\s*\/?>/', '$B', $str);	
	$str = preg_replace('/&nbsp;/', ' ', $str);	
	$str = strip_tags($str);
	
	if ($testing)
		return $str;

	// special symbols $
	$str = preg_replace('/&lt;(name|имя)&gt;/', '$N', $str);
	$str = preg_replace('/&lt;(race|раса)&gt;/', '$R', $str);	
	$str = preg_replace('/&lt;(класс|class)&gt;/', '$C', $str);	

	$str = preg_replace('/&amp;lt;(.*?)\/(.*?)&amp;gt;/', '$g$1:$2;', $str);	
	$str = preg_replace('/&lt;(.*?)\/(.*?)&gt;/', '$g$1:$2;', $str);

	$str = html_entity_decode($str);
	
	$str = preg_replace('/\s{2,}/', ' ', $str); // double space
	$str = preg_replace('/(\.{3})([а-яА-Яa-zA-Z0-9]+)/u', '$1 $2', $str); // war...is -> war... is
	$str = preg_replace('/\. \. \. \./', '...', $str); // . . . . -> ....
	$str = preg_replace('/(--|\s{2})/', ' - ', $str); // Hello--dima -> Hello - dima
	$str = preg_replace('/ $B/', '$B', $str);
	//return $conn->escape($str);
	return $str;
}

$regexes = 
[
	'/<h1 class="heading-size-1">(.*?)</' => ['Title','1'], // Title
	'/<h2 class="heading-size-3">(Description|Описание)<\/h2>(.*?)<h2 class="heading-size-3">/s' => ['Details','2'], // Details
	'/<h1 class="heading-size-1">.*?<\/h1>(.*?)<(?:script|table|div|h2)/m' => ['Objectives','1'], // Objectives
	'/(Completion|Завершено)<\/a><\/h2><div id=.*?-completion.*?">(.*?)<\/div>/' => ['OfferRewardText','2'], // OfferRewardText (Completion)
	'/(Progress|Прогресс).*?>(.*?)<(?:script|table|div|h2)/' => ['RequestItemsText','2'], // RequestItemsText (Progress)
	'/<p style="height:26px">&nbsp;<\/p><\/th><td><span>(.*?)<\/span>/' => ['EndText','1'], // EndText
];

if (!empty($entries))
	$sql = $entries;
else
	$sql = $conn->q("select entry from mangos_tbcdef.quest_template where entry not in (select entry from script.wowhead_quests)");

echo 'Start...'.PHP_EOL;

foreach($sql as $row) {
	if (!empty($entries))
		$entry = $row;
	else
		$entry = $row['entry'];

	foreach($links as $lang => $link) {
		$data = [];
		
		$data['Title'] = null;	
		$data['Details'] = null;	
		$data['Objectives'] = null;	
		$data['OfferRewardText'] = null;	
		$data['RequestItemsText'] = null;	
		$data['EndText'] = null;	
		$data['ObjectiveText1'] = null;	
		$data['ObjectiveText2'] = null;	
		$data['ObjectiveText3'] = null;	
		$data['ObjectiveText4'] = null;	

		while (!$html = file_get_html($link.$entry)) {
			echo "Can't get file_get_contents... sleeping".PHP_EOL;
			sleep(5);
		}

		if (
		preg_match('/This quest is no longer available within the game/',$html) || 
		preg_match('/Это задание более не доступно в игре./',$html) || 
		preg_match('/It may have been removed from the game./',$html) || 
		preg_match('/Возможно, его больше нет в игре./',$html)
		) {
			$conn->q("insert into script.wowhead_quests (entry,lang) values (?,?)",[$entry,$lang]);
			if ($conn->count !== 1)
				die("Die at $entry #1");
			continue;
		}

		foreach($regexes as $regex => $col_group) {
			$group = $col_group[1];
			$column = $col_group[0];
			
			if (preg_match($regex, $html, $matches)) {
				$data[$column] = convert($matches[$group]);
			}
			else if ($column == "OfferRewardText") {
				// OfferRewardText (Completion) IF EMPTY needed for 8316
				if (preg_match('/(Completion|Завершено)<\/h2>(.*?)<(?:script|table|div)/', $html, $matches2)) {
					$data[$column] = convert($matches2[2]);
				}
			}
		}

		// ObjectiveText
		$list = $html->find('div.text .icon-list tr');
		
		$i=1;
		foreach($list as $t) {
			$obj_text = $t->find('td a', 0);
			if ($obj_text)
				$data['ObjectiveText'.$i] = convert($obj_text->plaintext);
			
			++$i;
		}
		
		if (empty($data['Title']))
			continue;
		
		if (empty($entries)) {
			$conn->q("insert into script.wowhead_quests (entry,lang,Title,Details,Objectives,OfferRewardText,RequestItemsText,EndText,ObjectiveText1,ObjectiveText2,ObjectiveText3,ObjectiveText4) values (?,?,?,?,?,?,?,?,?,?,?,?)",[
				$entry,
				$lang,
				$data['Title'],
				$data['Details'],
				$data['Objectives'],
				$data['OfferRewardText'],
				$data['RequestItemsText'],
				$data['EndText'],
				$data['ObjectiveText1'],
				$data['ObjectiveText2'],
				$data['ObjectiveText3'],
				$data['ObjectiveText4'],	
			]);
			
			if ($conn->count !== 1)
				die("Die at $entry #2");
		} else {
			print_r($data);
		}
	}
	
	echo $entry.' ok'.PHP_EOL;
}


// EXECUTE BEFORE FUNCTION!!!

// update quest_template set ObjectiveText1='' where ObjectiveText1 is null;
// update quest_template set ObjectiveText2='' where ObjectiveText2 is null;
// update quest_template set ObjectiveText3='' where ObjectiveText3 is null;
// update quest_template set ObjectiveText4='' where ObjectiveText4 is null;

//var_dump($conn->getLastQuery());

// ----------- testing

//SELECT
//	GROUP_CONCAT(entry),
//	SUBSTRING( details, LOCATE( '$', details ) + 1, 1 ) AS v,
//	count(*) c 
//FROM
//	quest_template 
//WHERE
//	details REGEXP '\\$.*' 
//GROUP BY
//	v 
//ORDER BY
//	c;

// SELECT entry, details FROM wowhead_quests WHERE details REGEXP BINARY '^[a-z]' LIMIT 1;
// SELECT entry, details FROM wowhead_quests WHERE requestitemstext REGEXP BINARY '^[a-z]' LIMIT 1;
// SELECT entry, details FROM wowhead_quests WHERE title REGEXP BINARY '^[a-z]' LIMIT 1;

// SELECT
	// a.entry,
	// a.title,
	// a.offerrewardtext,
	// b.offerrewardtext
// FROM
	// mangos5_tbc.quest_template a
	// LEFT JOIN script.wowhead_quests b ON a.entry = b.entry 
// WHERE
	// a.offerrewardtext != b.offerrewardtext
	// AND b.lang = 'en';
	
	
// ----------- update
