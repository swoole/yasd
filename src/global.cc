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
#include "include/global.h"
#include "include/util.h"
#include "include/common.h"
#include "include/cmder_debugger.h"
#include "include/remote_debugger.h"

#include "./php_yasd.h"

yasd::Global *global = nullptr;

zend_function *get_cid_function = nullptr;

int64_t get_cid() {
    zval retval;

    if (!get_cid_function) {
        return 0;
    }

    get_cid_function->internal_function.handler(nullptr, &retval);

    return Z_LVAL(retval) == -1 ? 0 : Z_LVAL(retval);
}

namespace yasd {
Global::Global() {
    if (YASD_G(log_level) >= 0) {
        logger = new yasd::Logger("debug.log");
        logger->set_level(YASD_G(log_level));
    }

    breakpoints = new std::map<BREAKPOINT>();
    contexts = new std::map<int, Context *>();

    if (strncmp(YASD_G(debug_mode), "cmd", sizeof("cmd") - 1) == 0) {
        debugger = new CmderDebugger();
    } else if (strncmp(YASD_G(debug_mode), "remote", sizeof("remote") - 1) == 0) {
        debugger = new RemoteDebugger();
    } else {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_RED, "error debug mode");
        exit(255);
    }
}

Global::~Global() {
    delete debugger;
    debugger = nullptr;
    delete contexts;
    contexts = nullptr;
    delete breakpoints;
    breakpoints = nullptr;
    delete logger;

    yasd_zend_fci_cache_discard(onGreaterThanMilliseconds);
    efree(onGreaterThanMilliseconds);
    onGreaterThanMilliseconds = nullptr;
}

Context *Global::get_current_context() {
    yasd::Context *context = nullptr;
    int64_t cid = get_cid();

    auto iter = global->contexts->find(cid);

    if (iter != global->contexts->end()) {
        context = iter->second;

        // printf("get context: %p, strace: %p, cid: %lld\n", context, context->strace, cid);
    } else {
        context = new yasd::Context();
        context->cid = cid;
        global->contexts->insert(std::make_pair(cid, context));

        // printf("create context: %p, strace: %p, cid: %lld\n", context, context->strace, cid);
    }

    return context;
}
}  // namespace yasd
