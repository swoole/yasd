<?php

declare(strict_types=1);
/**
 * This file is part of Yasd.
 *
 * @contact  codinghuang@qq.com
 */
namespace Yasd;

class CmdClient
{
    public function start(string $testFile)
    {
        $descriptorspec = [
            0 => ['pipe', 'r'],  // stdin is a pipe that the child will read from
            1 => ['pipe', 'w'],  // stdout is a pipe that the child will write to
        ];

        $cwd = __DIR__;

        $process = proc_open("php -e {$testFile}", $descriptorspec, $pipes, $cwd);

        if (is_resource($process)) {
            fwrite($pipes[0], "r\n");
            fclose($pipes[0]);

            echo stream_get_contents($pipes[1]);
            fclose($pipes[1]);

            // It is important that you close any pipes before calling
            // proc_close in order to avoid a deadlock
            $return_value = proc_close($process);

            assert($return_value == 0);
        }
    }
}

! defined('BASE_PATH') && define('BASE_PATH', dirname(__DIR__, 2));

(new CmdClient)->start(BASE_PATH . DIRECTORY_SEPARATOR . 'test.php');
