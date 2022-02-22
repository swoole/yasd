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

#include "./php_yasd_cxx.h"
#include "include/dbgp.h"
#include "include/base64.h"
#include "include/util.h"
#include "include/zend_property_info.h"

YASD_EXTERN_C_BEGIN
#include "ext/standard/php_string.h"
YASD_EXTERN_C_END

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

void Dbgp::make_message(tinyxml2::XMLDocument *doc, yasd::Buffer *buffer) {
    // https://xdebug.org/docs/dbgp#response

    buffer->clear();

    tinyxml2::XMLPrinter printer;

    doc->Print(&printer);

    int size = printer.CStrSize() - 1 + sizeof("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n") - 1;
    buffer->append(std::to_string(size));
    buffer->append("\0", 1);
    buffer->append("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
    buffer->append(printer.CStr());
    buffer->append("\0", 1);
}

void Dbgp::get_response_doc(tinyxml2::XMLElement *root, const ResponseElement &response_element) {
    root->SetAttribute("xmlns", "urn:debugger_protocol_v1");
    root->SetAttribute("xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
    root->SetAttribute("command", response_element.cmd.c_str());
    root->SetAttribute("transaction_id", response_element.transaction_id);
}

void Dbgp::get_property_doc(tinyxml2::XMLElement *root, PropertyElement *property_element) {
    root->SetAttribute("type", property_element->type.c_str());
    if (property_element->name != "") {
        root->SetAttribute("name", property_element->name.c_str());
    }
    if (property_element->fullname != "") {
        root->SetAttribute("fullname", property_element->fullname.c_str());
    }

    while (Z_TYPE_P(property_element->value) == IS_INDIRECT) {
        property_element->value = Z_INDIRECT_P(property_element->value);
    }

    if (Z_TYPE_P(property_element->value) == IS_REFERENCE) {  // $GLOBALS is IS_REFERENCE
        property_element->value = &((property_element->value)->value.ref->val);
    }

    switch (Z_TYPE_P(property_element->value)) {
    case IS_TRUE:
        // zend_zval_type_name may return boolean, so we should convert to bool
        root->SetAttribute("type", "bool");
        root->InsertNewText("1")->SetCData(true);
        break;
    case IS_FALSE:
        // zend_zval_type_name may return boolean, so we should convert to bool
        root->SetAttribute("type", "bool");
        root->InsertNewText("0")->SetCData(true);
        break;
    case IS_NULL:
        break;
    case IS_LONG:
        // because the zend_zval_type_name function of PHP7.2 return 'integer'
        root->SetAttribute("type", "int");
        root->InsertNewText(std::to_string(Z_LVAL_P(property_element->value)).c_str())->SetCData(true);
        break;
    case IS_DOUBLE:
        root->InsertNewText(std::to_string(Z_DVAL_P(property_element->value)).c_str())->SetCData(true);
        break;
    case IS_STRING:
        get_zend_string_property_doc(root, *property_element);
        break;
    case IS_ARRAY:
        get_zend_array_child_property_doc(root, *property_element);
        break;
    case IS_OBJECT: {
        get_zend_object_child_property_doc(root, *property_element);
        break;
    }
    case IS_UNDEF:
        root->SetAttribute("type", "uninitialized");
        break;
    default:
        break;
    }
}

void Dbgp::get_zend_string_property_doc(tinyxml2::XMLElement *root, const PropertyElement &property_element) {
    root->SetAttribute("size", (uint64_t) Z_STRLEN_P(property_element.value));
    if (property_element.encoding) {
        root->SetAttribute("encoding", "base64");
        root->InsertNewText(
                base64_encode((unsigned char *) Z_STRVAL_P(property_element.value), Z_STRLEN_P(property_element.value))
                    .c_str())
            ->SetCData(true);
    } else {
        root->InsertNewText(Z_STRVAL_P(property_element.value))->SetCData(true);
    }
}

void Dbgp::get_zend_array_child_property_doc(tinyxml2::XMLElement *child, const PropertyElement &property_element) {
    zend_ulong num;
    zend_string *key;
    zval *val;
    zend_array *ht = Z_ARRVAL_P(property_element.value);
    int level = property_element.level;

    child->SetAttribute("children", ht->nNumOfElements > 0 ? "1" : "0");
    if (yasd_zend_hash_is_recursive(ht)) {
        child->SetAttribute("recursive", 1);
    } else {
        child->SetAttribute("numchildren", ht->nNumOfElements);
        if (level < YASD_G(depth)) {
            yasd_zend_hash_apply_protection_begin(ht);  // set recursive for $GLOBALS['GLOBALS']
            ZEND_HASH_FOREACH_KEY_VAL_IND(ht, num, key, val) {
                tinyxml2::XMLElement *property = child->InsertNewChildElement("property");
                std::string child_name = "";
                std::string slashe_child_name = "";
                std::string child_fullname = "";

                if (key == nullptr) {  // num key
                    child_name = std::to_string(num);
                    if (property_element.fullname != "") {
                        child_fullname = property_element.fullname + "[" + child_name + "]";
                    }
                } else {  // string key
                    child_name = ZSTR_VAL(key);
                    slashe_child_name = yasd::util::string::addslashes(std::string(ZSTR_VAL(key), ZSTR_LEN(key)));

                    if (property_element.fullname != "") {  // eval don't need fullname in phpstorm
                        child_fullname = property_element.fullname + "[\"" + slashe_child_name + "\"]";
                    }
                }

                level++;
                yasd::PropertyElement property_element;
                property_element.set_type(zend_zval_type_name(val))
                    .set_name(child_name)
                    .set_fullname(child_fullname)
                    .set_value(val)
                    .set_level(level)
                    .set_encoding(true);
                get_property_doc(property, &property_element);
                level--;
            }
            ZEND_HASH_FOREACH_END();
            yasd_zend_hash_apply_protection_end(ht);
        }
    }
}

void Dbgp::get_zend_object_child_property_doc(tinyxml2::XMLElement *child, const PropertyElement &property_element) {
    zend_string *class_name;
    zend_array *properties;
    class_name = Z_OBJCE_P(property_element.value)->name;
    zend_string *key;
    zval *val;

    // TODO(codinghuang): it seems that properties will be nullptr
    properties = yasd::util::get_properties(property_element.value);
    int level = property_element.level;

    child->SetAttribute("type", "object");
    child->SetAttribute("classname", ZSTR_VAL(class_name));
    child->SetAttribute("children", (properties && properties->nNumOfElements > 0) ? "1" : "0");

    if (UNEXPECTED(!properties)) {
        return;
    }

    std::vector<yasd::ZendPropertyInfo> summary_properties_info;

    if (yasd_zend_hash_is_recursive(properties)) {
        child->SetAttribute("recursive", 1);
    } else {
        child->SetAttribute("numchildren", properties ? properties->nNumOfElements : 0);
        if (level < YASD_G(depth)) {
            yasd_zend_hash_apply_protection_begin(properties);
            ZEND_HASH_FOREACH_KEY_VAL_IND(properties, num, key, val) {
                tinyxml2::XMLElement *property = child->InsertNewChildElement("property");
                std::string child_fullname;
                std::string child_name;

                child_name = yasd::util::get_property_name(key);
                if (property_element.fullname != "") {  // eval don't need fullname in phpstorm
                    child_fullname = property_element.fullname + "->" + child_name;
                }

                level++;

                yasd::PropertyElement property_element;
                property_element.set_type(zend_zval_type_name(val))
                    .set_name(child_name)
                    .set_fullname(child_fullname)
                    .set_value(val)
                    .set_level(level)
                    .set_encoding(true);
                get_property_doc(property, &property_element);

                if (level > YASD_G(depth)) {
                    child->DeleteChild(property);
                }
                level--;
            }
            ZEND_HASH_FOREACH_END();
            yasd_zend_hash_apply_protection_end(properties);
        }
    }
}
}  // namespace yasd
