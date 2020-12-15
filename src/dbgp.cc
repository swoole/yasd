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

#include "./php_yasd.h"
#include "include/dbgp.h"
#include "include/base64.h"
#include "include/common.h"
#include "include/util.h"
#include "include/zend_property_info.h"

#include <memory>

namespace yasd {
void Dbgp::get_init_event_doc(tinyxml2::XMLDocument *doc, const DbgpInitElement &init_element) {
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

void Dbgp::get_message_doc(tinyxml2::XMLDocument *doc,
                           const ResponseElement &response_element,
                           const MessageElement &message_element) {
    tinyxml2::XMLElement *root;
    tinyxml2::XMLElement *child;

    root = doc->NewElement("response");
    doc->LinkEndChild(root);
    yasd::Dbgp::get_response_doc(root, response_element);
    root->SetAttribute("status", "break");
    root->SetAttribute("reason", "ok");

    child = root->InsertNewChildElement("xdebug:message");
    std::string filename_ = "file://" + message_element.filename;
    child->SetAttribute("filename", filename_.c_str());
    child->SetAttribute("lineno", message_element.lineno);
}

std::string Dbgp::make_message(tinyxml2::XMLDocument *doc) {
    // https://xdebug.org/docs/dbgp#response

    std::string message = "";
    tinyxml2::XMLPrinter printer;

    doc->Print(&printer);

    int size = printer.CStrSize() - 1 + sizeof("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n") - 1;
    message = message + std::to_string(size);
    message += '\0';
    message += "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
    message += printer.CStr();
    message += '\0';

    return message;
}

void Dbgp::get_response_doc(tinyxml2::XMLElement *root, const ResponseElement &response_element) {
    root->SetAttribute("xmlns", "urn:debugger_protocol_v1");
    root->SetAttribute("xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
    root->SetAttribute("command", response_element.cmd.c_str());
    root->SetAttribute("transaction_id", response_element.transaction_id);
}

void Dbgp::get_property_doc(tinyxml2::XMLElement *child, const PropertyElement &property_element) {
    if (property_element.level > YASD_G(depth)) {
        return;
    }

    switch (Z_TYPE_P(property_element.value)) {
    case IS_TRUE:
        child->SetAttribute("type", "bool");
        child->InsertNewText("1")->SetCData(true);
        break;
    case IS_FALSE:
        child->SetAttribute("type", "bool");
        child->InsertNewText("0")->SetCData(true);
        break;
    case IS_NULL:
        child->SetAttribute("type", "null");
        break;
    case IS_LONG:
        child->SetAttribute("type", "int");
        child->InsertNewText(std::to_string(Z_LVAL_P(property_element.value)).c_str())->SetCData(true);
        break;
    case IS_DOUBLE:
        child->SetAttribute("type", "float");
        child->InsertNewText(std::to_string(Z_DVAL_P(property_element.value)).c_str())->SetCData(true);
        break;
    case IS_STRING:
        child->SetAttribute("type", "string");
        if (property_element.encoding) {
            child->SetAttribute("size", (uint64_t) Z_STRLEN_P(property_element.value));
            child->SetAttribute("encoding", "base64");
            child
                ->InsertNewText(base64_encode((unsigned char *) Z_STRVAL_P(property_element.value),
                                              Z_STRLEN_P(property_element.value))
                                    .c_str())
                ->SetCData(true);
        } else {
            child->InsertNewText(Z_STRVAL_P(property_element.value))->SetCData(true);
        }
        break;
    case IS_ARRAY:
        init_zend_array_element_xml_property_node(
            child, property_element.name, property_element.value, property_element.level, property_element.encoding);
        break;
    case IS_OBJECT: {
        init_zend_object_property_xml_property_node(
            child, property_element.name, property_element.value, property_element.level, property_element.encoding);
        break;
    }
    case IS_UNDEF:
        child->SetAttribute("type", "uninitialized");
        break;
    default:
        break;
    }
}

void Dbgp::init_zend_array_element_xml_property_node(
    tinyxml2::XMLElement *child, std::string name, zval *value, int level, bool encoding) {
    zend_ulong num;
    zend_string *key;
    zval *val;
    zend_array *ht = Z_ARRVAL_P(value);

    child->SetAttribute("type", "array");
    child->SetAttribute("children", ht->nNumOfElements > 0 ? "1" : "0");
    child->SetAttribute("numchildren", ht->nNumOfElements);
    if (yasd_zend_hash_is_recursive(ht)) {
        child->SetAttribute("recursive", 1);
    } else {
        ZEND_HASH_FOREACH_KEY_VAL_IND(ht, num, key, val) {
            tinyxml2::XMLElement *property = child->InsertNewChildElement("property");
            std::string fullname;
            std::string key_str;

            if (key == nullptr) {  // num key
                key_str = std::to_string(num);
                property->SetAttribute("name", num);
                fullname = name + "[" + std::to_string(num) + "]";
            } else {  // string key
                key_str = ZSTR_VAL(key);
                property->SetAttribute("name", ZSTR_VAL(key));
                fullname = name + "[" + ZSTR_VAL(key) + "]";
            }

            property->SetAttribute("type", zend_zval_type_name(val));
            property->SetAttribute("fullname", fullname.c_str());
            level++;

            yasd::PropertyElement property_element;
            property_element.set_name(key_str).set_value(val).set_level(level).set_encoding(true);
            get_property_doc(property, property_element);

            if (level > YASD_G(depth)) {
                child->DeleteChild(property);
            }
            level--;
        }
        ZEND_HASH_FOREACH_END();
    }
}

void Dbgp::init_zend_object_property_xml_property_node(
    tinyxml2::XMLElement *child, std::string name, zval *value, int level, bool encoding) {
    zend_string *class_name;
    zend_array *properties;
    class_name = Z_OBJCE_P(value)->name;
    zend_ulong num;
    zend_string *key;
    zval *val;
    properties = yasd::Util::get_properties(value);

    child->SetAttribute("type", "object");
    child->SetAttribute("classname", ZSTR_VAL(class_name));
    child->SetAttribute("children", properties->nNumOfElements > 0 ? "1" : "0");
    child->SetAttribute("numchildren", properties->nNumOfElements);

    std::vector<yasd::ZendPropertyInfo> summary_properties_info;

    // TODO(codinghuang): may we have a better way to get private properties
    void *property_info;
    ZEND_HASH_FOREACH_STR_KEY_PTR(&Z_OBJCE_P(value)->properties_info, key, property_info) {
        ZendPropertyInfo info;
        info.property_name = key;
        summary_properties_info.emplace_back(info);
    }
    ZEND_HASH_FOREACH_END();

    int i = 0;
    ZEND_HASH_FOREACH_KEY_VAL_IND(properties, num, key, val) {
        tinyxml2::XMLElement *property = child->InsertNewChildElement("property");
        std::string fullname;
        std::string key_str;

        ZendPropertyInfo info = summary_properties_info[i];
        key = info.property_name;

        key_str = ZSTR_VAL(key);
        property->SetAttribute("name", key_str.c_str());
        fullname = name + "->" + key_str;

        property->SetAttribute("fullname", fullname.c_str());
        property->SetAttribute("type", zend_zval_type_name(val));
        level++;

        yasd::PropertyElement property_element;
        property_element.set_name(key_str).set_value(val).set_level(level).set_encoding(true);
        get_property_doc(property, property_element);

        if (level > YASD_G(depth)) {
            child->DeleteChild(property);
        }
        level--;
        i++;
    }
    ZEND_HASH_FOREACH_END();
}
}  // namespace yasd
