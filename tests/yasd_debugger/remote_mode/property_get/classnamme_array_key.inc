<?php
namespace Bar\Banana;

class Foo
{
    public $arr = [];

    public function __construct()
    {
        $this->arr = [
            self::class => [1, 2, 3],
        ];
    }
}

$foo = new Foo();
return;
?>