<?php

namespace Bar;

use Swoole\Coroutine;

use function Swoole\Coroutine\run;

function test1() {
    echo 'test1' . PHP_EOL;
    test2();
}

function test2() {
    return;
    echo 'test2' . PHP_EOL;
}

function test3() {
    return [
        ['hello'],
        ['world'],
        [new Foo(1)],
    ];
}

// Suppose the business code calls a function to determine whether to end the loop, 
// but the business code always returns false
function test4() {
    return false;
}

class Foo3
{
    public $a;

    private $c;

    public function __construct()
    {
        $this->a = "hello";
        $this->c = ['a', 'b'];
    }
}

class Foo2
{
    public $a;

    public $b;

    private $foo3;

    /**
     * Map of entries that are already resolved.
     *
     * @var array
     */
    private $resolvedEntries = [];

    public function test()
    {
        return [
            ['hello'],
            ['world'],
            [new Foo(1)],
        ];
    }

    public function __construct()
    {
        $this->a = "hello";
        $this->resolvedEntries = [
            'aaa' => new Foo3(),
            'bbb' => [4, 5, 6],
        ];
        $this->foo3 = new Foo3();
    }
}

class Foo
{
    public $a;

    public $foo2;

    public $arr;

    public function test()
    {
        return [
            ['hello'],
            ['world'],
            [new Foo(1)],
        ];
    }

    public function __construct($a)
    {
        $this->arr = [
            self::class => [self::class => [self::class => 1]],
        ];
        $this->a = "hello";
        $this->foo2 = new Foo2();
    }
}
$b = true;
$b = false;
$i = 0;
$j = 1;
$j = 2;
$k = [];

$k = ['a' => ['b' => [1, 2, 3]]];
$k = [
    ['hello'],
    ['world'],
];

while (true) {
    if ($i++ == 5) {
        $k = ['hello' => 1, "world" => 1.1];
        $j = 5;
    }
    if ($i == 10) {
        $k = ['hello', 'world'];
        $j = 10;
        break;
    }
}

$a = 0;
while (true) {
    $a++;
    if (test4() || $a > 50000) {
        break;
    }
}

echo 'hello' . PHP_EOL;

require __DIR__ . '/header.php';

echo 'world' . PHP_EOL;

$foo = new Foo(2);

run(function () {
    Coroutine::create(function () {
        $i = 1;
        test1();
        Coroutine::yield();
        $i = 2;
        $a = ['a' => 1, 'b' => 2];
        var_dump($a);
    });

    Coroutine::create(function () {
        $i = 3;
        Coroutine::resume(2);
        $i = 4;
    });
});
