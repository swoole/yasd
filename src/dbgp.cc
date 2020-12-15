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

#include "include/dbgp.h"

#include <memory>

namespace yasd {
void Dbgp::init_response(tinyxml2::XMLDocument *doc, const DbgpInitElement &init_element) {
    // https://xdebug.org/docs/dbgp#connection-initialization

    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;

    root = doc->NewElement("init");
    doc->LinkEndChild(root);
    root->SetAttribute("xmlns", "urn:debugger_protocol_v1");
    root->SetAttribute("xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");

    child = doc->NewElement("engine");
    root->InsertEndChild(child);
    child->SetAttribute("version", init_element.debugger_version.c_str());
    child->InsertNewText(init_element.debugger_name.c_str())->SetCData(true);

    child = doc->NewElement("author");
    root->InsertEndChild(child);
    child->InsertNewText(init_element.author.c_str())->SetCData(true);

    child = doc->NewElement("url");
    root->InsertEndChild(child);
    child->InsertNewText(init_element.url.c_str())->SetCData(true);

    child = doc->NewElement("copyright");
    root->InsertEndChild(child);
    child->InsertNewText(init_element.copyright.c_str())->SetCData(true);

    root->SetAttribute("fileuri", init_element.fileuri.c_str());
    root->SetAttribute("language", init_element.language.c_str());
    root->SetAttribute("xdebug:language_version", init_element.language_version.c_str());
    root->SetAttribute("protocol_version", "1.0");
    root->SetAttribute("appid", init_element.appid.c_str());
    root->SetAttribute("idekey", init_element.idekey.c_str());
}

}  // namespace yasd
