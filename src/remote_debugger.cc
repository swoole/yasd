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

#include <iostream>

#include "include/global.h"
#include "include/remote_debugger.h"

#include "thirdparty/tinyxml2/tinyxml2.h"

#include "main/php.h"

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
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());

    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;

    root = doc->NewElement("init");
    doc->LinkEndChild(root);
    root->SetAttribute("xmlns", "urn:debugger_protocol_v1");
    root->SetAttribute("xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");

    child = doc->NewElement("engine");
    root->InsertEndChild(child);
    child->SetAttribute("version", "0.1.0");
    child->InsertNewText("Yasd")->SetCData(true);

    child = doc->NewElement("author");
    root->InsertEndChild(child);
    child->InsertNewText("Codinghuang")->SetCData(true);

    child = doc->NewElement("url");
    root->InsertEndChild(child);
    child->InsertNewText("https://github.com/swoole/yasd")->SetCData(true);

    child = doc->NewElement("copyright");
    root->InsertEndChild(child);
    child->InsertNewText("Copyright (c) 2020-2021 by Codinghuang")->SetCData(true);

    std::string fileuri = "file://" + std::string(global->entry_file);
    root->SetAttribute("fileuri", fileuri.c_str());
    root->SetAttribute("language", "PHP");
    root->SetAttribute("xdebug:language_version", PHP_VERSION);
    root->SetAttribute("protocol_version", "1.0");
    root->SetAttribute("appid", std::to_string(getpid()).c_str());
    root->SetAttribute("idekey", "hantaohuang");

    tinyxml2::XMLPrinter printer;
    doc->Print();
    doc->Print(&printer);

    std::string message = "";

    message =
        message + std::to_string(printer.CStrSize() + sizeof("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n") - 1);
    message += '\0';
    message += "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
    message += '\0';
    message += printer.CStr();

    ssize_t ret = send(sock, message.c_str(), message.length(), 0);
    printf("send: %ld\n", ret);
}

void RemoteDebugger::handle_request(const char *filename, int lineno) {}

}  // namespace yasd
