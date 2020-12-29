--TEST--
phpstorm: eval
--SKIPIF--
--FILE--
<?php

use Yasd\DbgpClient;

require dirname(__DIR__, 3) . '/Include/bootstrap.php';

$basename = basename(__FILE__, '.php');
$filename = realpath(dirname(__FILE__) . "/{$basename}.inc");

$code1 = base64_encode('$GLOBALS[\'IDE_EVAL_CACHE\'][\'ebaa8c82-8268-473a-9240-46cf7d077939\']=test()');
$code2 = base64_encode('$GLOBALS[\'IDE_EVAL_CACHE\'][\'9e3aafd1-aba1-4e1f-8bf0-45274347956a\']=$foo->test()');

$commands = [
    "breakpoint_set -t line -f file://{$filename} -n 78",
    'run',
    "eval -- {$code1}",
    "property_get -n \$GLOBALS['IDE_EVAL_CACHE']['ebaa8c82-8268-473a-9240-46cf7d077939'][0]", // test()[0]
    "eval -- {$code2}",
    "property_get -n \$GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][0]", // $foo->test()[0]
    "property_get -n \$GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][2]", // $foo->test()[2]
    "property_get -n GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][2][0]", // $foo->test()[2][0]
    "property_get -n GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][2][0]->foo2", // $foo->test()[2][0]->foo2
    'run',
    'stop',
];

$client = (new DbgpClient())->setCommands($commands)->setTestFile($filename)->start();

?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri=%s language="PHP" xdebug:language_version=%s protocol_version="1.0" appid=%s idekey=%s><engine version=%s><![CDATA[Yasd]]></engine><author><![CDATA[Codinghuang]]></author><url><![CDATA[https://github.com/swoole/yasd]]></url><copyright><![CDATA[Copyright (c) 2020-2021 by Codinghuang]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://%s -n 78
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="%s"/>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://%s" lineno="78"/></response>

-> eval -i 3 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJ2ViYWE4YzgyLTgyNjgtNDczYS05MjQwLTQ2Y2Y3ZDA3NzkzOSddPXRlc3QoKQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="3"><property type="array" children="1" numchildren="3"><property type="array" name="0" children="1" numchildren="1"/><property type="array" name="1" children="1" numchildren="1"/><property type="array" name="2" children="1" numchildren="1"/></property></response>

-> property_get -i 4 -n $GLOBALS['IDE_EVAL_CACHE']['ebaa8c82-8268-473a-9240-46cf7d077939'][0]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property type="array" name="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;ebaa8c82-8268-473a-9240-46cf7d077939&apos;][0]" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;ebaa8c82-8268-473a-9240-46cf7d077939&apos;][0]" children="1" numchildren="1"><property type="string" name="0" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;ebaa8c82-8268-473a-9240-46cf7d077939&apos;][0][0]" size="5" encoding="base64"><![CDATA[aGVsbG8=]]></property></property></response>

-> eval -i 5 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzllM2FhZmQxLWFiYTEtNGUxZi04YmYwLTQ1Mjc0MzQ3OTU2YSddPSRmb28tPnRlc3QoKQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="5"><property type="array" children="1" numchildren="3"><property type="array" name="0" children="1" numchildren="1"/><property type="array" name="1" children="1" numchildren="1"/><property type="array" name="2" children="1" numchildren="1"/></property></response>

-> property_get -i 6 -n $GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][0]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property type="array" name="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][0]" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][0]" children="1" numchildren="1"><property type="string" name="0" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][0][0]" size="5" encoding="base64"><![CDATA[aGVsbG8=]]></property></property></response>

-> property_get -i 7 -n $GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property type="array" name="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2]" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2]" children="1" numchildren="1"><property type="object" name="0" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]" classname="Foo" children="1" numchildren="2"/></property></response>

-> property_get -i 8 -n GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][2][0]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property type="object" name="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]" classname="Foo" children="1" numchildren="2"><property type="string" name="a" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;a" size="5" encoding="base64"><![CDATA[aGVsbG8=]]></property><property type="object" name="foo2" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2" classname="Foo2" children="1" numchildren="4"/></property></response>

-> property_get -i 9 -n GLOBALS['IDE_EVAL_CACHE']['9e3aafd1-aba1-4e1f-8bf0-45274347956a'][2][0]->foo2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property type="object" name="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2" classname="Foo2" children="1" numchildren="4"><property type="string" name="a" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2-&gt;a" size="5" encoding="base64"><![CDATA[aGVsbG8=]]></property><property type="null" name="b" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2-&gt;b"/><property type="object" name="foo3" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2-&gt;foo3" classname="Foo3" children="1" numchildren="2"/><property type="array" name="resolvedEntries" fullname="GLOBALS[&apos;IDE_EVAL_CACHE&apos;][&apos;9e3aafd1-aba1-4e1f-8bf0-45274347956a&apos;][2][0]-&gt;foo2-&gt;resolvedEntries" children="1" numchildren="2"/></property></response>

-> run -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="10" status="stopping" reason="ok"/>

-> stop -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stop" transaction_id="11" status="stopped" reason="ok"/>