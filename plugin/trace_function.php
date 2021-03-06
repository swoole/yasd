<?php

namespace Yasd;

printf("\e[32m%s\e[0m", "[yasd plugin] load trace function plugin success\n");

$functionInfo = [];

Api\onEnterFunction(function (FunctionStatus $functionStatus) {
    global $functionInfo;

    if ($functionStatus->functionName === '') {
        $functionStatus->functionName = 'main';
    }

    if (!isset($functionInfo[$functionStatus->functionName])) {
        $functionInfo[$functionStatus->functionName] = 0;
    }

    $functionInfo[$functionStatus->functionName]++;
    printf("enter function {$functionStatus->functionName}, call frequency: {$functionInfo[$functionStatus->functionName]}\n");
});
