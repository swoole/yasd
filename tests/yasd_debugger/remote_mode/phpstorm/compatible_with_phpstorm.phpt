--TEST--
phpstorm: compatible_with_phpstorm
--SKIPIF--
--FILE--
<?php

use Yasd\DbgpClient;

require dirname(__DIR__, 3) . '/Include/bootstrap.php';

$basename = basename(__FILE__, '.php');
$filename = realpath(dirname(__FILE__) . "/{$basename}.inc");

$commands = [
    'eval -- KHN0cmluZykoaW5pX2dldCgneGRlYnVnLmNvdmVyYWdlX2VuYWJsZScpLic7Jy5pbmlfZ2V0KCd4ZGVidWcucHJvZmlsZXJfZW5hYmxlJykuJzsnLmluaV9nZXQoJ3hkZWJ1Zy5yZW1vdGVfYXV0b3N0YXJ0JykuJzsnLmluaV9nZXQoJ3hkZWJ1Zy5yZW1vdGVfY29ubmVjdF9iYWNrJykuJzsnLmluaV9nZXQoJ3hkZWJ1Zy5yZW1vdGVfbW9kZScpKQ==',
    'run',
    'stop',
];

$client = (new DbgpClient())->setCommands($commands)->setTestFile($filename)->start();

?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri=%s language="PHP" xdebug:language_version=%s protocol_version="1.0" appid=%s idekey=%s><engine version=%s><![CDATA[Yasd]]></engine><author><![CDATA[Codinghuang]]></author><url><![CDATA[https://github.com/swoole/yasd]]></url><copyright><![CDATA[Copyright (c) 2020-2021 by Codinghuang]]></copyright></init>

-> eval -i 1 -- KHN0cmluZykoaW5pX2dldCgneGRlYnVnLmNvdmVyYWdlX2VuYWJsZScpLic7Jy5pbmlfZ2V0KCd4ZGVidWcucHJvZmlsZXJfZW5hYmxlJykuJzsnLmluaV9nZXQoJ3hkZWJ1Zy5yZW1vdGVfYXV0b3N0YXJ0JykuJzsnLmluaV9nZXQoJ3hkZWJ1Zy5yZW1vdGVfY29ubmVjdF9iYWNrJykuJzsnLmluaV9nZXQoJ3hkZWJ1Zy5yZW1vdGVfbW9kZScpKQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="1"><property type="string" size="11"><![CDATA[1;1;1;0;req]]></property></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="stopping" reason="ok"/>

-> stop -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stop" transaction_id="3" status="stopped" reason="ok"/>
