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

$ref1 = "hello" . time();
$ref2 = &$ref1;

var_dump(Yasd\Zval\getRefCount($undef));
var_dump(Yasd\Zval\getRefCount($integer));
var_dump(Yasd\Zval\getRefCount($string));
var_dump(Yasd\Zval\getRefCount($array));
var_dump(Yasd\Zval\getRefCount($object));
var_dump(Yasd\Zval\getRefCount($bool));
var_dump(\Yasd\Zval\getRefCount($ref2));

?>
--EXPECTF--
%s: Undefined %s
NULL
NULL
int(%d)
int(%d)
int(%d)
NULL
int(%d)