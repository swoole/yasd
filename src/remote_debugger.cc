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
#include <memory>

#include "include/global.h"
#include "include/util.h"
#include "include/common.h"
#include "include/remote_debugger.h"

#include "main/php.h"

namespace yasd {

void RemoteDebugger::init() {
    register_cmd_handler();

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

    send_doc(doc.get());

    int status;
    do {
        get_next_cmd();
        status = execute_cmd();
    } while (status != yasd::DebuggerModeBase::status::NEXT_OPLINE);
}

std::string RemoteDebugger::get_next_cmd() {
    ssize_t ret;
    char buffer[4096];

    ret = recv(sock, buffer, 4096, 0);
    if (ret == 0) {
        printf("connection closed\n");
        exit(255);
    }
    printf("recv: %ld\n", ret);
    std::string tmp(buffer, buffer + ret);
    last_cmd = tmp;
    return last_cmd;
}

int RemoteDebugger::execute_cmd() {
    std::cout << last_cmd << std::endl;

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    transaction_id = atoi(exploded_cmd[2].c_str());

    auto handler = find_cmd_handler(exploded_cmd[0]);

    if (!handler) {
        return FAILED;
    }

    return handler();
}

void RemoteDebugger::handle_request(const char *filename, int lineno) {
    // 316<?xml version="1.0" encoding="iso-8859-1"?>
    // <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run"
    // transaction_id="6" status="break" reason="ok"><xdebug:message
    // filename="file:///Users/hantaohuang/codeDir/cppCode/yasd/test.php" lineno="31"></xdebug:message></response>

    int status;

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    root->SetAttribute("xmlns", "urn:debugger_protocol_v1");
    root->SetAttribute("xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
    root->SetAttribute("command", "run");
    root->SetAttribute("transaction_id", transaction_id);
    root->SetAttribute("status", "break");
    root->SetAttribute("reason", "ok");

    child = root->InsertNewChildElement("xdebug:message");
    std::string filename_ = "file://" + std::string(filename);
    child->SetAttribute("filename", filename_.c_str());
    child->SetAttribute("lineno", lineno);

    send_doc(doc.get());

    do {
        global->do_next = false;
        global->do_step = false;
        global->do_finish = false;

        get_next_cmd();
        status = execute_cmd();
    } while (status != yasd::DebuggerModeBase::status::NEXT_OPLINE);
}

ssize_t RemoteDebugger::send_doc(tinyxml2::XMLDocument *doc) {
    ssize_t ret;
    std::string message = make_message(doc);

    std::cout << message << std::endl;

    ret = send(sock, message.c_str(), message.length(), 0);
    printf("send: %ld\n", ret);

    return ret;
}

std::string RemoteDebugger::make_message(tinyxml2::XMLDocument *doc) {
    std::string message = "";
    tinyxml2::XMLPrinter printer;

    doc->Print(&printer);

    message =
        message + std::to_string(printer.CStrSize() + sizeof("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n") - 1);
    message += '\0';
    message += "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
    message += '\0';
    message += printer.CStr();

    return message;
}

void RemoteDebugger::init_response_xml_root_node(tinyxml2::XMLElement *root, std::string cmd) {
    root->SetAttribute("xmlns", "urn:debugger_protocol_v1");
    root->SetAttribute("xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
    root->SetAttribute("command", cmd.c_str());
    root->SetAttribute("transaction_id", transaction_id);
}

void RemoteDebugger::init_local_variables_xml_child_node(tinyxml2::XMLElement *root) {
    tinyxml2::XMLElement *child;

    unsigned int i = 0;
    HashTable *defined_vars;

    zend_op_array *op_array = &EG(current_execute_data)->func->op_array;

    while (i < (unsigned int) op_array->last_var) {
        child = root->InsertNewChildElement("property");
        zend_string *var_name = op_array->vars[i];
        child->SetAttribute("name", ZSTR_VAL(var_name));
        child->SetAttribute("fullname", ZSTR_VAL(var_name));

        zval *var = yasd::Util::find_variable(ZSTR_VAL(var_name));

        if (!var) {
            child->SetAttribute("type", "uninitialized");
        }

        i++;
    }
}

// breakpoint_list -i 1
int RemoteDebugger::parse_breakpoint_list_cmd() {
    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");
    if (exploded_cmd[0] != "breakpoint_list") {
        return yasd::DebuggerModeBase::FAILED;
    }
    if (exploded_cmd[1] != "-i") {
        return yasd::DebuggerModeBase::FAILED;
    }

    // 189<?xml version="1.0" encoding="iso-8859-1"?>
    // <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug"
    // command="breakpoint_list" transaction_id="1"></response>

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());

    tinyxml2::XMLElement *root;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    init_response_xml_root_node(root, "breakpoint_list");

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

// breakpoint_set -i 2 -t line -f file:///Users/hantaohuang/codeDir/cppCode/yasd/test.php -n 31
int RemoteDebugger::parse_breakpoint_set_cmd() {
    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");
    if (exploded_cmd[0] != "breakpoint_set") {
        return yasd::DebuggerModeBase::FAILED;
    }
    if (exploded_cmd[1] != "-i") {
        return yasd::DebuggerModeBase::FAILED;
    }

    if (exploded_cmd[3] != "-t") {
        return yasd::DebuggerModeBase::FAILED;
    }

    if (exploded_cmd[4] == "exception") {
        return parse_breakpoint_set_exception_cmd();
    }

    std::string file_url = exploded_cmd[6];
    file_url.substr(7, file_url.length() - 7);
    std::string filename = file_url.substr(7, file_url.length() - 7);
    int lineno = atoi(exploded_cmd[8].c_str());

    auto iter = global->breakpoints->find(filename);

    if (iter != global->breakpoints->end()) {
        iter->second.insert(lineno);
    } else {
        std::set<int> lineno_set;
        lineno_set.insert(lineno);
        global->breakpoints->insert(std::make_pair(filename, lineno_set));
    }

    yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "set breakpoint at %s:%d", filename.c_str(), lineno);

    // 203<?xml version="1.0" encoding="iso-8859-1"?>
    // <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set"
    // transaction_id="2" id="217190001"></response>

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    init_response_xml_root_node(root, "breakpoint_set");
    root->SetAttribute("id", breakpoint_admin_add());

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

// breakpoint_set -i 5 -t exception -x *
int RemoteDebugger::parse_breakpoint_set_exception_cmd() {
    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    init_response_xml_root_node(root, "breakpoint_set");
    root->SetAttribute("id", breakpoint_admin_add());

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_run_cmd() {
    global->is_running = true;

    return yasd::DebuggerModeBase::NEXT_OPLINE;
}

int RemoteDebugger::parse_stack_get_cmd() {
    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;

    // <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get"
    // transaction_id="7">
    // <stack where="{main}" level="0" type="file" filename="file:///Users/hantaohuang/codeDir/cppCode/yasd/test.php"
    // lineno="31"></stack>
    // </response>

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    init_response_xml_root_node(root, "stack_get");
    root->SetAttribute("id", breakpoint_admin_add());

    std::string fileuri = "file://" + std::string(yasd::Util::get_executed_filename());

    child = root->InsertNewChildElement("stack");
    child->SetAttribute("where", "{main}");
    child->SetAttribute("level", "0");
    child->SetAttribute("type", "file");
    child->SetAttribute("filename", fileuri.c_str());
    child->SetAttribute("lineno", yasd::Util::get_executed_file_lineno());

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_context_names_cmd() {
    // 329<?xml version="1.0" encoding="iso-8859-1"?>
    // <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_names"
    // transaction_id="8">
    //     <context name="Locals" id="0"></context>
    //     <context name="Superglobals" id="1"></context>
    //     <context name="User defined constants" id="2"></context>
    // </response>

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;
    int id = 0;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    init_response_xml_root_node(root, "context_names");
    root->SetAttribute("id", breakpoint_admin_add());

    child = root->InsertNewChildElement("context");
    child->SetAttribute("name", "Locals");
    child->SetAttribute("id", id++);

    child = root->InsertNewChildElement("context");
    child->SetAttribute("name", "Superglobals");
    child->SetAttribute("id", id++);

    child = root->InsertNewChildElement("context");
    child->SetAttribute("name", "User defined constants");
    child->SetAttribute("id", id++);

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_context_get_cmd() {
    // context_get -i 10 -d 0 -c 1
    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");
    int context_id;
    if (exploded_cmd[0] != "context_get") {
        return yasd::DebuggerModeBase::FAILED;
    }
    context_id = atoi(exploded_cmd[6].c_str());

    // 465<?xml version="1.0" encoding="iso-8859-1"?>
    // <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get"
    // transaction_id="9" context="0">
    //     <property name="$foo" fullname="$foo" type="uninitialized"></property>
    //     <property name="$i" fullname="$i" type="uninitialized"></property>
    //     <property name="$j" fullname="$j" type="uninitialized"></property>
    //     <property name="$k" fullname="$k" type="uninitialized"></property>
    // </response>

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    init_response_xml_root_node(root, "context_get");
    root->SetAttribute("context", 0);

    if (context_id == LOCALS) {
        init_local_variables_xml_child_node(root);
    }

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

void RemoteDebugger::register_cmd_handler() {
    handlers.push_back(std::make_pair("breakpoint_list", std::bind(&RemoteDebugger::parse_breakpoint_list_cmd, this)));
    handlers.push_back(std::make_pair("breakpoint_set", std::bind(&RemoteDebugger::parse_breakpoint_set_cmd, this)));
    handlers.push_back(std::make_pair("run", std::bind(&RemoteDebugger::parse_run_cmd, this)));
    handlers.push_back(std::make_pair("stack_get", std::bind(&RemoteDebugger::parse_stack_get_cmd, this)));
    handlers.push_back(std::make_pair("context_names", std::bind(&RemoteDebugger::parse_context_names_cmd, this)));
    handlers.push_back(std::make_pair("context_get", std::bind(&RemoteDebugger::parse_context_get_cmd, this)));
}

std::function<int()> RemoteDebugger::find_cmd_handler(std::string cmd) {
    for (auto &&kv : handlers) {
        if (kv.first == get_full_name(cmd)) {
            return kv.second;
        }
    }
    return nullptr;
}

std::string RemoteDebugger::get_full_name(std::string sub_cmd) {
    for (auto &&kv : handlers) {
        if (yasd::Util::is_match(sub_cmd, kv.first)) {
            return kv.first;
        }
    }
    return "unknown cmd";
}

int RemoteDebugger::breakpoint_admin_add() {
    global->breakpoint_count++;
    return ((getpid() & 0x1ffff) * 10000) + global->breakpoint_count;
}

}  // namespace yasd
