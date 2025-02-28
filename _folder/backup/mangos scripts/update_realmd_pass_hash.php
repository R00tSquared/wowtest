<?php
require_once __DIR__ . '/composer/vendor/autoload.php';
use Laizerox\Wowemu\SRP\UserClient;

// connect
$user = '1';
$password = '2';

$err_acc = [];

try {
	$conn = new PDO("mysql:host=localhost;dbname=realmd_tbc", $user, $password);
	if (!$conn)
		throw new PDOException("Can't connect to database");

	$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

	$stmt1 = $conn->query("SELECT account_id,username,pass_hash FROM realmd.account WHERE pass_hash!='null'"); # where account_id between 170180 and 170190
	$columns = $stmt1->fetchAll(PDO::FETCH_ASSOC);
	
	$i=0;
	foreach ($columns as $col) {
		// Access individual column properties
		$account_id = $col['account_id'];
		$pass_hash = $col['pass_hash'];
		$username = $col['username'];
		
		if ($pass_hash == 'null') {
			$err_acc[] = $account_id;
			continue;
		}

		$client = new UserClient($username);
		$client->sha1raw = hex2bin($pass_hash);
		
		if (!$client->sha1raw) {
			throw new PDOException("Error at hash ".$pass_hash);
		}

		$salt = strtoupper($client->generateSalt()); 
		$verifier = strtoupper($client->generateVerifier($password));
		$stmt = $conn->prepare("UPDATE realmd_tbc.account SET v='$verifier', s='$salt' WHERE id='$account_id' and username='$username'");

		$stmt->execute();
		
		if ($stmt->rowCount() == 0)
			$err_acc[] = $account_id;
		
		++$i;
		if ($i % 1000)
			echo $i.PHP_EOL;
	}
	
	echo 'Done! Good accounts: '.$i.PHP_EOL;
	echo 'Errors accounts:'.PHP_EOL;
	foreach ($err_acc as $acc)
		echo $acc.PHP_EOL;
	
} catch (PDOException $e) {
    echo PHP_EOL."Connection failed: " . $e->getMessage().PHP_EOL;
	
	echo 'Errors accounts:'.PHP_EOL;
	foreach ($err_acc as $acc)
		echo $acc.PHP_EOL;	
}
