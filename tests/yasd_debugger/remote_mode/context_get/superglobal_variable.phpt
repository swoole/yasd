--TEST--
context_get: superglobal_variable
--SKIPIF--
--FILE--
<?php

use Yasd\DbgpClient;

require dirname(__DIR__, 3) . '/Include/bootstrap.php';

$basename = basename(__FILE__, '.php');
$filename = realpath(dirname(__FILE__) . "/{$basename}.inc");

$commands = [
    "breakpoint_set -t line -f file://{$filename} -n 3",
    'run',
    'context_get -d 0 -c 1',
    'run',
    'stop',
];

$client = (new DbgpClient())->setCommands($commands)->setTestFile($filename)->start();

?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri=%s language="PHP" xdebug:language_version=%s protocol_version="1.0" appid=%s idekey=%s><engine version=%s><![CDATA[Yasd]]></engine><author><![CDATA[Codinghuang]]></author><url><![CDATA[https://github.com/swoole/yasd]]></url><copyright><![CDATA[Copyright (c) 2020-2021 by Codinghuang]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://%s -n 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="%s"/>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file:///Users/hantaohuang/codeDir/cppCode/yasd/tests/yasd_debugger/remote_mode/context_get/superglobal_variable.inc" lineno="3"/></response>

-> context_get -i 3 -d 0 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0">%s

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="stopping" reason="ok"/>

-> stop -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stop" transaction_id="5" status="stopped" reason="ok"/>
