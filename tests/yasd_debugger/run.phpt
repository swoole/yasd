--TEST--
Check if yasd is loaded
--SKIPIF--
<?php
if (!extension_loaded('yasd')) {
	echo 'skip, not install yasd extension';
}
?>
--FILE--
<?php

use Yasd\DbgpClient;

require dirname(__DIR__, 1) . '/Include/bootstrap.php';

$filename = realpath(dirname(__FILE__) . '/run.inc');

$commands = [
    'run',
    'stop'
];

$client = (new DbgpClient)->setCommands($commands)->setTestFile($filename)->start();
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri=%s language="PHP" xdebug:language_version="7.4.10" protocol_version="1.0" appid=%s idekey=%s><engine version="0.1.0"><![CDATA[Yasd]]></engine><author><![CDATA[Codinghuang]]></author><url><![CDATA[https://github.com/swoole/yasd]]></url><copyright><![CDATA[Copyright (c) 2020-2021 by Codinghuang]]></copyright></init>

-> run -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="1" status="stopping" reason="ok"/>

-> stop -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stop" transaction_id="2" status="stopped" reason="ok"/>
