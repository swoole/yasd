<?php

function test()
{
    return [
        ['hello'],
        ['world'],
        [new Foo(1)],
    ];
}

class Foo3
{
    public $a;

    private $c;

    public function __construct()
    {
        $this->a = 'hello';
        $this->c = ['a', 'b'];
    }
}

class Foo2
{
    public $a;

    public $b;

    private $foo3;

    private $resolvedEntries = [];

    public function __construct()
    {
        $this->a = 'hello';
        $this->resolvedEntries = [
            'aaa' => new Foo3(),
            'bbb' => [4, 5, 6],
        ];
        $this->foo3 = new Foo3();
    }

    public function test()
    {
        return [
            ['hello'],
            ['world'],
            [new Foo(1)],
        ];
    }
}

class Foo
{
    public $a;

    public $foo2;

    public function __construct($a)
    {
        $this->a = 'hello';
        $this->foo2 = new Foo2();
    }

    public function test()
    {
        return [
            ['hello'],
            ['world'],
            [new Foo(1)],
        ];
    }
}

$foo = new Foo(2);
return;
