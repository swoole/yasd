<?php

echo 'execute init_file success' . PHP_EOL;

// Yasd\Api\setBreakpoint(__DIR__ . DIRECTORY_SEPARATOR . 'test.php', 107);
// Yasd\Api\setMaxExecutedOplineNum(2000000);
Yasd\Api\setRemoteHost(gethostbyname('localhost'));

\Yasd\Api\onGreaterThanMilliseconds(10, function (string $functionName, int $executeTime) {
    if (!$functionName) {
        $functionName = 'main';
    }

    echo "function name: $functionName, execute time: $executeTime" . PHP_EOL;
});
