--TEST--
phpstorm: idekey
--SKIPIF--
--FILE--
<?php

use Yasd\DbgpClient;

require dirname(__DIR__, 3) . '/Include/bootstrap.php';

$basename = basename(__FILE__, '.php');
$filename = realpath(dirname(__FILE__) . "/{$basename}.inc");

$code1 = base64_encode("ini_get('xdebug.ide_key')");

$commands = [
    "eval -- {$code1}",
    "run",
    "stop"
];

$client = (new DbgpClient())->setCommands($commands)->setTestFile($filename)->start();

?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri=%s language="PHP" xdebug:language_version=%s protocol_version="1.0" appid=%s idekey="yasd-test"><engine version=%s><![CDATA[Yasd]]></engine><author><![CDATA[Codinghuang]]></author><url><![CDATA[https://github.com/swoole/yasd]]></url><copyright><![CDATA[Copyright (c) 2020-2021 by Codinghuang]]></copyright></init>

-> eval -i 1 -- aW5pX2dldCgneGRlYnVnLmlkZV9rZXknKQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="1"><property type="bool"><![CDATA[0]]></property></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="stopping" reason="ok"/>

-> stop -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stop" transaction_id="3" status="stopped" reason="ok"/>
