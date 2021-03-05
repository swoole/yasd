<?php

namespace Yasd;

use SplStack;

class Node
{
    /**
     * @var Node
     */
    public $parentNode;

    /**
     * @var Node[]
     */
    public $childNodes;

    /**
     * @var int
     */
    public $totalExecuteTime;

    /**
     * @var string
     */
    public $functionName;

    /**
     * @var string
     */
    public $parentFunctionName;

    public function __construct(string $functionName, int $totalExecuteTime, ?string $parentFunctionName)
    {
        $this->functionName = $functionName;
        $this->totalExecuteTime = $totalExecuteTime;
        $this->parentFunctionName = $parentFunctionName;
    }
}

echo 'execute init_file success' . PHP_EOL;

// Api\setBreakpoint(__DIR__ . DIRECTORY_SEPARATOR . 'test.php', 107);
// Api\setMaxExecutedOplineNum(2000000);
Api\setRemoteHost(gethostbyname('localhost'));

$nodeStack = new SplStack();
$nodeStack->push(null);

Api\onGreaterThanMilliseconds(10, function (FunctionStatus $functionStatus) {
    global $callRelationship;
    global $nodeStack;

    if ($functionStatus->functionName === '') {
        $functionStatus->functionName = 'main';
    }

    if ($functionStatus->parentFunctionName === '') {
        $functionStatus->parentFunctionName = 'main';
    }

    if ($functionStatus->parentFunctionName !== null) {
        $callRelationship[$functionStatus->parentFunctionName] = $functionStatus->functionName;
    }

    /**
     * @var Node
     */
    $node = new Node($functionStatus->functionName, $functionStatus->executeTime, $functionStatus->parentFunctionName);

    while (!$nodeStack->isEmpty()) {
        /**
         * @var Node
         */
        $lastNode = $nodeStack->top();

        if ($lastNode && $lastNode->parentFunctionName == $functionStatus->functionName) {
            $node->childNodes[] = $lastNode;
            $nodeStack->pop();
        } else {
            break;
        }
    }

    $nodeStack->push($node);

    $childFunctionExecuteTime = 0;

    if ($node->childNodes) {
        foreach ($node->childNodes as $childNode) {
            $childFunctionExecuteTime += $childNode->totalExecuteTime;
        }
    }

    $selfExecuteTime = $functionStatus->executeTime - $childFunctionExecuteTime;

    echo "functionName: $functionStatus->functionName, selfExecuteTime: $selfExecuteTime, totalExecuteTime: $functionStatus->executeTime\n";
});
