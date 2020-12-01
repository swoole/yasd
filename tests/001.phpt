--TEST--
Check if yasd is loaded
--SKIPIF--
<?php
if (!extension_loaded('yasd')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "yasd" is available';
?>
--EXPECT--
The extension "yasd" is available
