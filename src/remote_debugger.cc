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

#include "./php_yasd.h"
#include "include/global.h"
#include "include/util.h"
#include "include/common.h"
#include "include/base64.h"
#include "include/remote_debugger.h"
#include "include/dbgp.h"
#include "include/zend_property_info.h"

#include <boost/algorithm/string.hpp>

namespace yasd {

RemoteDebugger::RemoteDebugger() {
    buffer = new yasd::Buffer(512);
}

RemoteDebugger::~RemoteDebugger() {
    close(sock);
    delete buffer;
}

void RemoteDebugger::init() {
    register_cmd_handler();

    struct sockaddr_in ide_address;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }

    ide_address.sin_family = AF_INET;
    ide_address.sin_port = htons(YASD_G(remote_port));

    if (inet_pton(AF_INET, YASD_G(remote_host), &ide_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *) &ide_address, sizeof(ide_address)) < 0) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_YELLOW, "[yasd] %s", strerror(errno));
        return;
    }

    send_init_event_message();

    int status;
    do {
        get_next_cmd();
        status = execute_cmd();
    } while (status != yasd::DebuggerModeBase::status::NEXT_OPLINE);
}

std::string RemoteDebugger::get_next_cmd() {
    ssize_t ret;
    char buffer[1024];
    char *p = buffer;

    // The IDE may send multiple commands, so we need to determine the delimiter.
    do {
        ret = recv(sock, p, 1, 0);
        if (ret == 0) {
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_YELLOW, "[yasd] recv command error, connection closed");
            zend_bailout();
        }
        if (ret < 0) {
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_YELLOW, "[yasd] recv command error, %s", strerror(errno));
            zend_bailout();
        }
    } while ((*p != '\0') && p++);

    last_cmd = std::string(buffer, buffer + (p - buffer));
    if (global->logger) {
        global->logger->put(yasd::LogLevel::DEBUG, last_cmd.c_str(), last_cmd.length());
    }
    return last_cmd;
}

int RemoteDebugger::execute_cmd() {
    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);

    transaction_id = atoi(yasd::Util::get_option_value(exploded_cmd, "-i").c_str());

    auto handler = find_cmd_handler(exploded_cmd[0]);

    if (!handler) {
        // printf("not found handler\n");
        return FAILED;
    }

    // printf("found handler\n");

    return handler();
}

void RemoteDebugger::handle_request(const char *filename, int lineno) {
    int status;

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    yasd::ResponseElement response_element;
    yasd::MessageElement message_element;
    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);

    response_element.set_cmd(exploded_cmd[0]).set_transaction_id(transaction_id);
    message_element.set_filename(filename).set_lineno(lineno);

    yasd::Dbgp::get_message_doc(doc.get(), response_element, message_element);
    send_doc(doc.get());

    do {
        global->do_next = false;
        global->do_step = false;
        global->do_finish = false;

        get_next_cmd();
        status = execute_cmd();
    } while (status != yasd::DebuggerModeBase::status::NEXT_OPLINE);
}

void RemoteDebugger::handle_stop() {
    if (!global->is_running) {
        return;
    }

    int status;
    std::vector<std::string> exploded_cmd;

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd(exploded_cmd[0]).set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("status", "stopping");
    root->SetAttribute("reason", "ok");

    send_doc(doc.get());

    need_bailout = false;

    do {
        get_next_cmd();
        status = execute_cmd();
    } while (status != yasd::DebuggerModeBase::status::NEXT_OPLINE);
}

ssize_t RemoteDebugger::send_init_event_message() {
    // https://xdebug.org/docs/dbgp#connection-initialization

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());

    yasd::DbgpInitElement init_element;

    init_element.set_appid(std::to_string(getpid()))
        .set_author("Codinghuang")
        .set_copyright("Copyright (c) 2020-2021 by Codinghuang")
        .set_debugger_name("Yasd")
        .set_debugger_version(PHP_YASD_VERSION)
        .set_fileuri("file://" + std::string(yasd::Util::get_executed_filename()))
        .set_idekey("hantaohuang")
        .set_language("PHP")
        .set_language_version(PHP_VERSION)
        .set_url("https://github.com/swoole/yasd");

    yasd::Dbgp::get_init_event_doc(doc.get(), init_element);

    return send_doc(doc.get());
}

ssize_t RemoteDebugger::send_doc(tinyxml2::XMLDocument *doc) {
    ssize_t ret;
    yasd::Dbgp::make_message(doc, buffer);

    if (global->logger) {
        global->logger->put(yasd::LogLevel::DEBUG, buffer->value(), buffer->get_length());
    }

    ret = send(sock, buffer->value(), buffer->get_length(), 0);

    return ret;
}

void RemoteDebugger::init_local_variables_xml_child_node(tinyxml2::XMLElement *root) {
    // https://xdebug.org/docs/dbgp#properties-variables-and-values

    tinyxml2::XMLElement *child;

    unsigned int i = 0;

    zend_op_array *op_array = &EG(current_execute_data)->func->op_array;

    while (i < (unsigned int) op_array->last_var) {
        child = root->InsertNewChildElement("property");
        zend_string *var_name = op_array->vars[i];
        std::string name = "$" + std::string(ZSTR_VAL(var_name));
        std::string fullname = std::string(ZSTR_VAL(var_name));

        zval *var = yasd::Util::find_variable(ZSTR_VAL(var_name));

        yasd::PropertyElement property_element;
        property_element.set_type(zend_zval_type_name(var))
            .set_name(name)
            .set_fullname(ZSTR_VAL(var_name))
            .set_value(var);
        yasd::Dbgp::get_property_doc(child, &property_element);

        i++;
    }

    if (Z_TYPE(EG(current_execute_data)->This) == IS_OBJECT) {
        child = root->InsertNewChildElement("property");

        yasd::PropertyElement property_element;
        property_element.set_type(zend_zval_type_name(&EG(current_execute_data)->This))
            .set_name("$this")
            .set_fullname("this")
            .set_value(&EG(current_execute_data)->This);
        yasd::Dbgp::get_property_doc(child, &property_element);
    }
}

void RemoteDebugger::init_superglobal_variables_xml_child_node(tinyxml2::XMLElement *root) {
    tinyxml2::XMLElement *child;
    zend_string *key;
    zval *val;

    ZEND_HASH_FOREACH_STR_KEY_VAL(&EG(symbol_table), key, val) {
        std::string key_str = std::string(ZSTR_VAL(key));

        // filter local variable
        if (Z_TYPE_P(val) == IS_INDIRECT || key_str == "argc" || key_str == "argv") {
            continue;
        }

        child = root->InsertNewChildElement("property");
        std::string name = "$" + key_str;
        std::string fullname = "GLOBALS['" + key_str + "'";

        yasd::PropertyElement property_element;
        property_element.set_type(zend_zval_type_name(val)).set_name(name).set_fullname(key_str).set_value(val);
        yasd::Dbgp::get_property_doc(child, &property_element);
    }
    ZEND_HASH_FOREACH_END();
}

void RemoteDebugger::init_user_defined_constant_variables_xml_child_node(tinyxml2::XMLElement *root) {
    // https://xdebug.org/docs/dbgp#properties-variables-and-values

    zend_constant *val;
    void *tmp;

    tinyxml2::XMLElement *child;

    ZEND_HASH_FOREACH_PTR(EG(zend_constants), tmp) {
        val = reinterpret_cast<zend_constant *>(tmp);
        zval *zval_value = &val->value;

        if (!val->name) {
            continue;
        }

        if (YASD_ZEND_CONSTANT_MODULE_NUMBER(val) != PHP_USER_CONSTANT) {
            continue;
        }

        child = root->InsertNewChildElement("property");
        child->SetAttribute("facet", "constant");

        yasd::PropertyElement property_element;
        property_element.set_type(zend_zval_type_name(zval_value))
            .set_name(ZSTR_VAL(val->name))
            .set_fullname(ZSTR_VAL(val->name))
            .set_value(zval_value);
        yasd::Dbgp::get_property_doc(child, &property_element);
    }
    ZEND_HASH_FOREACH_END();
    return;
}

int RemoteDebugger::parse_feature_set_cmd() {
    // https://xdebug.org/docs/dbgp#feature-set
    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    std::string feature = yasd::Util::get_option_value(exploded_cmd, "-n");

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("feature_set").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("feature", feature.c_str());
    root->SetAttribute("success", 0);

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_stdout_cmd() {
    // https://xdebug.org/docs/dbgp#stdout-stderr

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("stdout").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("success", 0);

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_status_cmd() {
    // https://xdebug.org/docs/dbgp#status

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("status").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("status", "starting");
    root->SetAttribute("reason", "ok");

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_eval_cmd() {
    // https://xdebug.org/docs/dbgp#eval

    zval ret_zval;
    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);

    if (exploded_cmd[3] != "--") {
        return yasd::DebuggerModeBase::FAILED;
    }

    std::string eval_str = exploded_cmd[4];

    // std::cout << "eval_str: " << eval_str << std::endl;

    eval_str = base64_decode(eval_str);

    // std::cout << "base64_decode eval_str: " << eval_str << std::endl;

    yasd::Util::eval(const_cast<char *>(eval_str.c_str()), &ret_zval, const_cast<char *>("yasd://debug-eval"));

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("eval").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    child = root->InsertNewChildElement("property");

    yasd::PropertyElement property_element = {};
    property_element.set_type(zend_zval_type_name(&ret_zval)).set_value(&ret_zval).set_encoding(true);
    yasd::Dbgp::get_property_doc(child, &property_element);

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_breakpoint_list_cmd() {
    // https://xdebug.org/docs/dbgp#id7

    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);
    if (exploded_cmd[0] != "breakpoint_list") {
        return yasd::DebuggerModeBase::FAILED;
    }
    if (exploded_cmd[1] != "-i") {
        return yasd::DebuggerModeBase::FAILED;
    }

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    response_element.set_cmd("breakpoint_list").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_breakpoint_set_cmd() {
    // https://xdebug.org/docs/dbgp#id3

    std::string breakpoint_type;
    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);
    if (exploded_cmd[0] != "breakpoint_set") {
        return yasd::DebuggerModeBase::FAILED;
    }

    breakpoint_type = yasd::Util::get_option_value(exploded_cmd, "-t");

    if (breakpoint_type == "line") {
        parse_breakpoint_set_line_cmd(exploded_cmd);
    } else if (breakpoint_type == "conditional") {
        parse_breakpoint_set_condition_line_cmd(exploded_cmd);
    }

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("breakpoint_set").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("id", breakpoint_admin_add());

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

void RemoteDebugger::parse_breakpoint_set_line_cmd(const std::vector<std::string> &exploded_cmd) {
    // https://xdebug.org/docs/dbgp#id3

    std::string file_url = yasd::Util::get_option_value(exploded_cmd, "-f");
    int lineno = atoi(yasd::Util::get_option_value(exploded_cmd, "-n").c_str());

    file_url.substr(sizeof("file://") - 1, file_url.length() - (sizeof("file://") - 1));
    std::string filename = file_url.substr(7, file_url.length() - 7);

    auto iter = global->breakpoints->find(filename);

    if (iter != global->breakpoints->end()) {
        iter->second.insert(lineno);
    } else {
        std::set<int> lineno_set;
        lineno_set.insert(lineno);
        global->breakpoints->insert(std::make_pair(filename, lineno_set));
    }
}

void RemoteDebugger::parse_breakpoint_set_condition_line_cmd(const std::vector<std::string> &exploded_cmd) {
    int lineno = atoi(yasd::Util::get_option_value(exploded_cmd, "-n").c_str());
    std::string condition = base64_decode(yasd::Util::get_option_value(exploded_cmd, "--"));

    parse_breakpoint_set_line_cmd(exploded_cmd);

    auto iter = global->breakpoint_conditions.find(lineno);

    if (iter != global->breakpoint_conditions.end()) {
        iter->second.insert(lineno, condition);
    } else {
        std::pair<BREAKPOINT_CONDITION> breakpoint_condition = std::make_pair(lineno, condition);
        global->breakpoint_conditions.insert(breakpoint_condition);
    }
}

void RemoteDebugger::parse_breakpoint_set_exception_cmd(const std::vector<std::string> &exploded_cmd) {
    // https://xdebug.org/docs/dbgp#id3

    return;
}

int RemoteDebugger::parse_run_cmd() {
    global->is_running = true;

    return yasd::DebuggerModeBase::NEXT_OPLINE;
}

int RemoteDebugger::parse_stack_get_cmd() {
    // https://xdebug.org/docs/dbgp#stack-get

    yasd::Context *context = global->get_current_context();

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("stack_get").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("id", breakpoint_admin_add());

    child = root->InsertNewChildElement("stack");
    const char *tmp = yasd::Util::get_executed_function_name();
    child->SetAttribute("where", tmp);
    child->SetAttribute("level", "0");
    child->SetAttribute("type", "file");

    std::string fileuri = "file://" + std::string(yasd::Util::get_executed_filename());
    child->SetAttribute("filename", fileuri.c_str());
    child->SetAttribute("lineno", yasd::Util::get_executed_file_lineno());

    for (auto iter = context->strace->rbegin(); iter != context->strace->rend(); ++iter) {
        // yasd::Util::printfln_info(YASD_ECHO_GREEN, "%s:%d", (*iter)->filename.c_str(), (*iter)->lineno);
        child = root->InsertNewChildElement("stack");
        child->SetAttribute("where", (*iter)->function_name.c_str());
        child->SetAttribute("level", (*iter)->level);
        child->SetAttribute("type", "file");

        std::string fileuri = "file://" + (*iter)->filename;
        child->SetAttribute("filename", fileuri.c_str());
        child->SetAttribute("lineno", (*iter)->lineno);
    }

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_context_names_cmd() {
    // https://xdebug.org/docs/dbgp#context-names

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;
    yasd::ResponseElement response_element;

    int id = 0;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    response_element.set_cmd("context_names").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
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
    // https://xdebug.org/docs/dbgp#context-get

    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);
    int context_id;
    if (exploded_cmd[0] != "context_get") {
        return yasd::DebuggerModeBase::FAILED;
    }
    context_id = atoi(yasd::Util::get_option_value(exploded_cmd, "-c").c_str());

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    response_element.set_cmd("context_get").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("context", 0);

    if (context_id == LOCALS) {
        // Locals
        init_local_variables_xml_child_node(root);
    } else if (context_id == SUPER_GLOBALS) {
        // Superglobals
        init_superglobal_variables_xml_child_node(root);
    } else {
        // User defined constants
        init_user_defined_constant_variables_xml_child_node(root);
    }

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_property_get_cmd() {
    // https://xdebug.org/docs/dbgp#property-get-property-set-property-value

    std::vector<std::string> exploded_cmd;

    boost::split(exploded_cmd, last_cmd, boost::is_any_of(" "), boost::token_compress_on);
    std::string fullname;
    zval *property;

    if (exploded_cmd[0] != "property_get") {
        return yasd::DebuggerModeBase::FAILED;
    }

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);

    response_element.set_cmd("property_get").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);

    fullname = yasd::Util::get_option_value(exploded_cmd, "-n");

    // vscode has double quotes, but PhpStorm does not
    if (fullname.front() == '"') {
        fullname.erase(0, 1);
    }

    if (fullname.back() == '"') {
        fullname.pop_back();
    }

    // phpstorm eval expression will use $GLOBALS
    if (fullname.front() == '$') {
        fullname.erase(0, 1);
    }

    property = yasd::Util::fetch_zval_by_fullname(fullname);

    child = root->InsertNewChildElement("property");

    yasd::PropertyElement property_element;
    property_element.set_type(zend_zval_type_name(property))
        .set_name(fullname)
        .set_fullname(fullname)
        .set_value(property);
    yasd::Dbgp::get_property_doc(child, &property_element);

    send_doc(doc.get());

    return yasd::DebuggerModeBase::RECV_CMD_AGAIN;
}

int RemoteDebugger::parse_stop_cmd() {
    global->is_running = false;

    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    tinyxml2::XMLElement *root;
    yasd::ResponseElement response_element;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    response_element.set_cmd("stop").set_transaction_id(transaction_id);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("status", "stopped");
    root->SetAttribute("reason", "ok");

    send_doc(doc.get());

    if (need_bailout) {
        zend_bailout();
    }

    return yasd::DebuggerModeBase::NEXT_OPLINE;
}

void RemoteDebugger::register_cmd_handler() {
    handlers.emplace_back(std::make_pair("feature_set", std::bind(&RemoteDebugger::parse_feature_set_cmd, this)));
    handlers.emplace_back(std::make_pair("stdout", std::bind(&RemoteDebugger::parse_stdout_cmd, this)));
    handlers.emplace_back(std::make_pair("status", std::bind(&RemoteDebugger::parse_status_cmd, this)));
    handlers.emplace_back(std::make_pair("eval", std::bind(&RemoteDebugger::parse_eval_cmd, this)));
    handlers.emplace_back(
        std::make_pair("breakpoint_list", std::bind(&RemoteDebugger::parse_breakpoint_list_cmd, this)));
    handlers.emplace_back(std::make_pair("breakpoint_set", std::bind(&RemoteDebugger::parse_breakpoint_set_cmd, this)));
    handlers.emplace_back(std::make_pair("run", std::bind(&RemoteDebugger::parse_run_cmd, this)));
    handlers.emplace_back(std::make_pair("stack_get", std::bind(&RemoteDebugger::parse_stack_get_cmd, this)));
    handlers.emplace_back(std::make_pair("property_get", std::bind(&RemoteDebugger::parse_property_get_cmd, this)));
    handlers.emplace_back(std::make_pair("context_names", std::bind(&RemoteDebugger::parse_context_names_cmd, this)));
    handlers.emplace_back(std::make_pair("context_get", std::bind(&RemoteDebugger::parse_context_get_cmd, this)));
    handlers.emplace_back(std::make_pair("step_over", std::bind(&RemoteDebugger::parse_step_over_cmd, this)));
    handlers.emplace_back(std::make_pair("step_into", std::bind(&RemoteDebugger::parse_step_into_cmd, this)));
    handlers.emplace_back(std::make_pair("step_out", std::bind(&RemoteDebugger::parse_step_out_cmd, this)));
    handlers.emplace_back(std::make_pair("stop", std::bind(&RemoteDebugger::parse_stop_cmd, this)));
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
