--TEST--
opcode: getOpcodeByName
--SKIPIF--
--FILE--
<?php

var_dump(Yasd\getOpcodeByName('ZEND_NOP'));
var_dump(Yasd\getOpcodeByName('ZEND_ADD'));
var_dump(Yasd\getOpcodeByName('ZEND_SUB'));
var_dump(Yasd\getOpcodeByName('ZEND_MUL'));
var_dump(Yasd\getOpcodeByName('ZEND_DIV'));
?>
--EXPECTF--
int(0)
int(1)
int(2)
int(3)
int(4)
