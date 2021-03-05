<?php

namespace Yasd\Api
{
    /**
     * @param string $host
     * @return bool
     */
    function setRemoteHost(string $host) { }

    /**
     * @param string $fileAbsolutePath
     * @param int $lineno
     * @return bool
     */
    function setBreakpoint(string $fileAbsolutePath, int $lineno) { }

    /**
     * @param string $num
     * @return bool
     */
    function setMaxExecutedOplineNum(int $num) { }

    /**
     * @param int $milliseconds
     * @param callable $callback
     * @return bool
     */
    function onGreaterThanMilliseconds(int $milliseconds, callable $callback) { }
}

namespace Yasd
{
    class FunctionStatus
    {
        /**
         * @var string
         */
        public $functionName;

        /**
         * @var int
         */
        public $executeTime;

        /**
         * @var string
         */
        public $parentFunctionName;
    }
}
