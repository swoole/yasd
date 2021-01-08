--TEST--
zval: getRefCount
--SKIPIF--
--FILE--
<?php

$integer = 1;
$string = "yasd";
$array = ['a' => ['b' => [1, 2, 3]]];
$object = new stdClass;
$bool = true;

var_dump(Yasd\Zval\getRefCount($undef));
var_dump(Yasd\Zval\getRefCount($integer));
var_dump(Yasd\Zval\getRefCount($string));
var_dump(Yasd\Zval\getRefCount($array));
var_dump(Yasd\Zval\getRefCount($object));
var_dump(Yasd\Zval\getRefCount($bool));
?>
--EXPECTF--
Notice: Undefined variable: undef %s
NULL
NULL
NULL
int(%d)
int(%d)
NULL