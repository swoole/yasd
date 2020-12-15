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
#pragma once

#include <string>

#include "thirdparty/tinyxml2/tinyxml2.h"

namespace yasd {

class DbgpInitElement {
 public:
    std::string debugger_name;
    std::string debugger_version;
    std::string fileuri;
    std::string language;
    std::string language_version;
    std::string appid;
    std::string idekey;

    std::string author;
    std::string url;
    std::string copyright;

    DbgpInitElement &set_debugger_name(std::string _debugger_name) {
        debugger_name = _debugger_name;
        return *this;
    }

    DbgpInitElement &set_debugger_version(std::string _debugger_version) {
        debugger_version = _debugger_version;
        return *this;
    }

    DbgpInitElement &set_fileuri(std::string _fileuri) {
        fileuri = _fileuri;
        return *this;
    }

    DbgpInitElement &set_language(std::string _language) {
        language = _language;
        return *this;
    }

    DbgpInitElement &set_language_version(std::string _language_version) {
        language_version = _language_version;
        return *this;
    }

    DbgpInitElement &set_idekey(std::string _idekey) {
        idekey = _idekey;
        return *this;
    }

    DbgpInitElement &set_appid(std::string _appid) {
        appid = _appid;
        return *this;
    }

    DbgpInitElement &set_author(std::string _author) {
        author = _author;
        return *this;
    }

    DbgpInitElement &set_url(std::string _url) {
        url = _url;
        return *this;
    }

    DbgpInitElement &set_copyright(std::string _copyright) {
        copyright = _copyright;
        return *this;
    }
};

class Dbgp {
 public:
    Dbgp() {}
    ~Dbgp() {}

    static void init_response(tinyxml2::XMLDocument *doc, const DbgpInitElement &init_element);
};
}  // namespace yasd
