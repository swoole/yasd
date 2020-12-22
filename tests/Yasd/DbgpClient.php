<?php

declare(strict_types=1);
/**
 * This file is part of Yasd.
 *
 * @contact  codinghuang@qq.com
 */
namespace Yasd;

class DbgpClient
{
    /**
     * @var string
     */
    protected $testFile;

    /**
     * @var string[]
     */
    protected $commands;

    /**
     * @var int
     */
    protected $transaction_id = 0;

    public function setTestFile(string $testFile): DbgpClient
    {
        $this->testFile = $testFile;

        return $this;
    }

    public function setCommands(array $commands): DbgpClient
    {
        $this->commands = $commands;

        return $this;
    }

    public function readLength($conn): int
    {
        $length = 0;
        while ("\0" !== ($char = fgetc($conn))) {
            if ($char === false) {
                echo "read a false for transaction_id {$this->transaction_id}" . PHP_EOL;
                return null;
            }
            if (! is_numeric($char)) {
                echo "read a non-number for transaction_id {$this->transaction_id}" . PHP_EOL;
                return null;
            }
            $length = $length * 10 + (int) $char;
        }
        return $length;
    }

    public function readContent($conn, $length): string
    {
        $content = '';
        while (0 < $length) {
            $data = fread($conn, $length);
            if ($data === false) {
                echo "read a false for {$this->transaction_id}" . PHP_EOL;
                return null;
            }
            $length -= strlen($data);
            $content .= $data;
        }
        $char = fgetc($conn);
        if ($char !== "\0") {
            echo 'must end with \0' . PHP_EOL;
        }

        return $content;
    }

    public function doRead($conn)
    {
        stream_set_timeout($conn, 1);

        $transaction_id = null;
        $length = $this->readLength($conn);
        $content = $this->readContent($conn, $length);

        echo $content, "\n\n";

        $matches = [];
        if (preg_match('@transaction_id="(?P<transaction_id>\d+)"@', $content, $matches)) {
            $transaction_id = $matches['transaction_id'] ?? null;
        }

        assert((int) $transaction_id == $this->transaction_id);
        ++$this->transaction_id;
    }

    public function sendCommand($conn, $command)
    {
        // add transaction_id
        $exploded_cmd = explode(' ', $command, 2);
        if (count($exploded_cmd) == 1) {
            $command = $exploded_cmd[0] . " -i {$this->transaction_id}";
        } else {
            $command = $exploded_cmd[0] . " -i {$this->transaction_id} " . $exploded_cmd[1];
        }

        echo '-> ', $command, "\n";
        fwrite($conn, $command . "\0");
    }

    public function start()
    {
        $descriptorspec = [
            0 => ['pipe', 'r'],  // stdin is a pipe that the child will read from
            1 => ['pipe', 'w'],  // stdout is a pipe that the child will write to
        ];

        $cwd = __DIR__;

        $socket = @stream_socket_server('tcp://0.0.0.0:9000', $errno, $errstr);

        $process = proc_open("php -e {$this->testFile}", $descriptorspec, $pipes, $cwd);

        $conn = @stream_socket_accept($socket, 20);

        // read init event message
        $this->doRead($conn);

        foreach ($this->commands as $command) {
            $this->sendCommand($conn, $command);
            $this->doRead($conn);
        }

        fclose($pipes[0]);
        fclose($pipes[1]);

        $return_value = proc_close($process);

        assert($return_value == 0);
    }
}
