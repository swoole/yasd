--TEST--
property_get: classnamme_array_key
--SKIPIF--
--FILE--
<?php

use Yasd\DbgpClient;

require dirname(__DIR__, 3) . '/Include/bootstrap.php';

$basename = basename(__FILE__, '.php');
$filename = realpath(dirname(__FILE__) . "/{$basename}.inc");

$commands = [
    "breakpoint_set -t line -f file://{$filename} -n 17",
    'run',
    'property_get -n "foo->arr["Bar\\\\\\\\Banana\\\\\\\\Foo"]"',
    'property_get -n foo->arr["Bar\\\\Banana\\\\Foo"]',
    'run',
    'stop',
];

$client = (new DbgpClient())->setCommands($commands)->setTestFile($filename)->start();

?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri=%s language="PHP" xdebug:language_version=%s protocol_version="1.0" appid=%s idekey=%s><engine version=%s><![CDATA[Yasd]]></engine><author><![CDATA[Codinghuang]]></author><url><![CDATA[https://github.com/swoole/yasd]]></url><copyright><![CDATA[Copyright (c) 2020-2021 by Codinghuang]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://%s -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="%s"/>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://%s" lineno="17"/></response>

-> property_get -i 3 -n "foo->arr["Bar\\\\Banana\\\\Foo"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property type="array" name="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;]" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;]" children="1" numchildren="3"><property type="int" name="0" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;][0]"><![CDATA[1]]></property><property type="int" name="1" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;][1]"><![CDATA[2]]></property><property type="int" name="2" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;][2]"><![CDATA[3]]></property></property></response>

-> property_get -i 4 -n foo->arr["Bar\\Banana\\Foo"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property type="array" name="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;]" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;]" children="1" numchildren="3"><property type="int" name="0" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;][0]"><![CDATA[1]]></property><property type="int" name="1" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;][1]"><![CDATA[2]]></property><property type="int" name="2" fullname="foo-&gt;arr[&quot;Bar\\Banana\\Foo&quot;][2]"><![CDATA[3]]></property></property></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="stopping" reason="ok"/>

-> stop -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stop" transaction_id="6" status="stopped" reason="ok"/>