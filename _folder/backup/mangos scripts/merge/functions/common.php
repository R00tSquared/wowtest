<?php

function dieIfEmpty($rowcount,$table) {
	$msg = "$table rowcount is $rowcount";
	
	if ($rowcount == 0)
		abort($msg);
	
	echo info($msg);
}

function dieIfNotEmpty($rowcount,$table) {
	if ($rowcount > 0)
		abort("$table is not empty, having $rowcount rows");
}

function echoIfNotEmpty($rowcount,$table,$column) {
	if ($rowcount > 0)
		warning("$table has $rowcount empty locales for column $column");
}

function usage() {
	abort('Usage: /run.php [3|5] /path/config.php');
}

function title($text)
{
	echo colored($text, 'purple');
}

function success($text)
{
	echo colored($text, 'green');
}

function info($text)
{
	echo colored($text, 'white');
}

function warning($text)
{
	echo colored($text, 'orange');
}

function abort($text)
{
	echo colored($text, 'red');
	exit;
}

function colored($text, $color) {
    $colorCode = "";

    switch ($color) {
        case "red":
            $colorCode = "\033[0;31m";
            break;
        case "green":
            $colorCode = "\033[0;32m";
            break;
        case "purple":
            $colorCode = "\033[1;35m";
            break;
        case "orange":
            $colorCode = "\033[0;33m";
            break;	
        case "white":
            $colorCode = "\033[1;37m";
            break;			
        default:
            // If the color is not recognized, default to no color
            $colorCode = "\033[0m";
            break;
    }

    $resetCode = "\033[0m";
    $coloredText = $colorCode . $text . $resetCode;
    return $coloredText . PHP_EOL;
}