<?php

echo 'execute init_file success' . PHP_EOL;
Yasd\Api\setBreakpoint(__DIR__ . DIRECTORY_SEPARATOR . 'test.php', 107);
Yasd\Api\setMaxExecutedOplineNum(2000000);
Yasd\Api\setRemoteHost(gethostbyname('host.docker.internal'));
