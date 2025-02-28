<?php
error_reporting(E_ALL);
ini_set('display_errors', '1');

// dbc files containing linebreaks!
// use regex replace
// \r\n(^(?![0-9]{1,5}\b)(.*)) -> " $1" --- dbcdata.txt
// ,\s*$ -> empty --- dbcdata.txt
// (?<!\\)''(?!') -> null --- output file

error_reporting(E_ALL);
ini_set('display_errors', '1');

$file = file_get_contents('dbcdata.txt');
//$file = '"Death\'s Edge",dfg';

function removeCommaInsideQuotes($matches) {
    return str_replace(",", "", $matches[0]);
}

// Use preg_replace_callback to remove commas inside quotes
$file = preg_replace_callback('/"([^"]*)"/', 'removeCommaInsideQuotes', $file);

function remove2($matches) {
    return str_replace("'", "\'", $matches[0]);
}

// Use preg_replace_callback to remove commas inside quotes
$file = preg_replace_callback('/"([^"]*)"/', 'remove2', $file);

$lines = explode(PHP_EOL, $file);

foreach($lines as $line) {
	if (empty($line))
		continue;
	
	$line = rtrim($line, ',');
	
	$data = str_getcsv($line, ',');
	$sql = "INSERT INTO spell_dbc select ".'\''.implode("','", $data).'\';';
	echo $sql.PHP_EOL;
}

// alter table spell_dbc add column `ManaCost` int(10) unsigned NOT NULL DEFAULT 0 after powertype;
// alter table spell_dbc add column `ManaPerSecondPerLevel` int(10) unsigned NOT NULL DEFAULT 0 after manapersecond;

// alter table spell_dbc drop column id64;
// alter table spell_dbc drop column id65;
// alter table spell_dbc drop column id124;
// alter table spell_dbc drop column SpellNameFlag;
// alter table spell_dbc drop column `RankFlags`;
// alter table spell_dbc drop column `Description`;
// alter table spell_dbc drop column `id163`;
// alter table spell_dbc drop column `id164`;
// alter table spell_dbc drop column `id165`;
// alter table spell_dbc drop column `id166`;
// alter table spell_dbc drop column `id167`;
// alter table spell_dbc drop column `id168`;
// alter table spell_dbc drop column `id169`;
// alter table spell_dbc drop column `id170`;
// alter table spell_dbc drop column `id171`;
// alter table spell_dbc drop column `id172`;
// alter table spell_dbc drop column `id173`;
// alter table spell_dbc drop column `id174`;
// alter table spell_dbc drop column `id175`;
// alter table spell_dbc drop column `id176`;
// alter table spell_dbc drop column `id177`;
// alter table spell_dbc drop column `DescriptionFlags`;
// alter table spell_dbc drop column `ToolTip`;
// alter table spell_dbc drop column id180;
// alter table spell_dbc drop column id181;
// alter table spell_dbc drop column id182;
// alter table spell_dbc drop column id183;
// alter table spell_dbc drop column id184;
// alter table spell_dbc drop column id185;
// alter table spell_dbc drop column id186;
// alter table spell_dbc drop column id187;
// alter table spell_dbc drop column id188;
// alter table spell_dbc drop column id189;
// alter table spell_dbc drop column id190;
// alter table spell_dbc drop column id191;
// alter table spell_dbc drop column id192;
// alter table spell_dbc drop column id193;
// alter table spell_dbc drop column id194;
// alter table spell_dbc drop column `ToolTipFlags`;
// alter table spell_dbc drop column id202;
// alter table spell_dbc add column `IsServerSide` int(10) unsigned NOT NULL DEFAULT 0 after SchoolMask;
// alter table spell_dbc add column `AttributesServerside` int(10) unsigned NOT NULL DEFAULT 0 after IsServerSide;

// update spell_dbc set Attributes = CONV(REPLACE(Attributes,'0x',''),16,10);
// update spell_dbc set AttributesEx = CONV(REPLACE(AttributesEx,'0x',''),16,10);
// update spell_dbc set AttributesEx2 = CONV(REPLACE(AttributesEx2,'0x',''),16,10);
// update spell_dbc set AttributesEx3 = CONV(REPLACE(AttributesEx3,'0x',''),16,10);
// update spell_dbc set AttributesEx4 = CONV(REPLACE(AttributesEx4,'0x',''),16,10);
// update spell_dbc set AttributesEx5 = CONV(REPLACE(AttributesEx5,'0x',''),16,10);
// update spell_dbc set AttributesEx6 = CONV(REPLACE(AttributesEx6,'0x',''),16,10);
// update spell_dbc set Stances = CONV(REPLACE(Stances,'0x',''),16,10);
// update spell_dbc set StancesNot = CONV(REPLACE(StancesNot,'0x',''),16,10);
// update spell_dbc set Targets = CONV(REPLACE(Targets,'0x',''),16,10);
// update spell_dbc set TargetAuraStateNot = CONV(REPLACE(TargetAuraStateNot,'0x',''),16,10);
// update spell_dbc set procFlags = CONV(REPLACE(procFlags,'0x',''),16,10);
// update spell_dbc set EquippedItemClass = CONV(REPLACE(EquippedItemClass,'0x',''),16,10);
// update spell_dbc set EffectBaseDice1 = CONV(REPLACE(EffectBaseDice1,'0x',''),16,10);
// update spell_dbc set EffectChainTarget2 = CONV(REPLACE(EffectChainTarget2,'0x',''),16,10);
// update spell_dbc set spellPriority = CONV(REPLACE(spellPriority,'0x',''),16,10);
// update spell_dbc set StartRecoveryTime = CONV(REPLACE(StartRecoveryTime,'0x',''),16,10);
// update spell_dbc set SpellFamilyFlags = CONV(REPLACE(SpellFamilyFlags,'0x',''),16,10);
// update spell_dbc set PreventionType = CONV(REPLACE(PreventionType,'0x',''),16,10);
// update spell_dbc set RequiredAuraVision = CONV(REPLACE(RequiredAuraVision,'0x',''),16,10);
// update spell_dbc set SchoolMask = CONV(REPLACE(SchoolMask,'0x',''),16,10);
// update spell_dbc set StartRecoveryCategory = CONV(REPLACE(StartRecoveryCategory,'0x',''),16,10);


// Speed
// Reagent2
// EffectDicePerLevel2
// EffectPointsPerComboPoint2
// MinFactionId
// MinReputation