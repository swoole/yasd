--TEST--
yasd_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('yasd')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = yasd_test1();

var_dump($ret);
?>
--EXPECT--
The extension yasd is loaded and working!
NULL
