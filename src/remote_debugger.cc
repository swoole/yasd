/*
  +----------------------------------------------------------------------+
  | Yasd                                                                 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: codinghuang  <codinghuang@qq.com>                            |
  +----------------------------------------------------------------------+
*/

#include "include/remote_debugger.h"

namespace yasd {

void RemoteDebugger::init() {
    struct sockaddr_in ide_address;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }

    ide_address.sin_family = AF_INET;
    ide_address.sin_port = htons(8000);

    if (inet_pton(AF_INET, "127.0.0.1", &ide_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *) &ide_address, sizeof(ide_address)) < 0) {
        perror("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
}

void RemoteDebugger::handle_request(const char *filename, int lineno) {}

}  // namespace yasd
