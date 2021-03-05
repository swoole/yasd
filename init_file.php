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

// Yasd\Api\setBreakpoint(__DIR__ . DIRECTORY_SEPARATOR . 'test.php', 107);
// Yasd\Api\setMaxExecutedOplineNum(2000000);
\Yasd\Api\setRemoteHost(gethostbyname('localhost'));

$nodeStack = new SplStack();
$nodeStack->push(null);

\Yasd\Api\onGreaterThanMilliseconds(10, function (string $functionName, int $executeTime, ?string $parentFunctionName) {
    global $callRelationship;
    global $nodeStack;

    if ($functionName === '') {
        $functionName = 'main';
    }

    if ($parentFunctionName === '') {
        $parentFunctionName = 'main';
    }

    if ($parentFunctionName !== null) {
        $callRelationship[$parentFunctionName] = $functionName;
    }

    /**
     * @var Node
     */
    $node = new Node($functionName, $executeTime, $parentFunctionName);

    while (!$nodeStack->isEmpty()) {
        $lastNode = $nodeStack->top();

        if ($lastNode && $lastNode->parentFunctionName == $functionName) {
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

    $selfExecuteTime = $executeTime - $childFunctionExecuteTime;

    echo "functionName: $functionName, selfExecuteTime: $selfExecuteTime, totalExecuteTime: $executeTime\n";
});
