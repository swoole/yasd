PHP_ARG_ENABLE([yasd],
[whether to enable yasd support],
[AS_HELP_STRING([--enable-yasd],
    [Enable yasd support])],
[no])

AC_DEFUN([YASD_CHECK_CXX_LIB], [
    AC_LANG_PUSH([C++])
    LIBNAME=$1
    AC_MSG_CHECKING([for boost])
    AC_TRY_COMPILE(
        [ #include $2 ],
        [],
        [ AC_MSG_RESULT(yes) ],
        [ AC_MSG_ERROR([lib $LIBNAME not found.  Try: install $LIBNAME library]) ]
    )
    AC_LANG_POP([C++])
])

PHP_ARG_ENABLE(yasd-dev, whether to enable yasd developer build flags,
[  --enable-yasd-dev       yasd: Enable developer flags],, no)

AC_MSG_CHECKING([if compiling with clang])
AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([], [[
        #ifndef __clang__
            not clang
        #endif
    ]])],
    [CLANG=yes], [CLANG=no]
)
AC_MSG_RESULT([$CLANG])

if test "$CLANG" = "yes"; then
    CFLAGS="$CFLAGS -std=gnu89"
fi

if test "$PHP_YASD" != "no"; then
    AC_DEFINE(HAVE_YASD, 1, [ Have yasd support ])

    YASD_CHECK_CXX_LIB([boost], [<boost/algorithm/string/constants.hpp>])

    if test "$PHP_YASD_DEV" = "yes"; then
        AX_CHECK_COMPILE_FLAG(-Wbool-conversion,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wbool-conversion")
        AX_CHECK_COMPILE_FLAG(-Wignored-qualifiers,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wignored-qualifiers")
        AX_CHECK_COMPILE_FLAG(-Wduplicate-enum,                 _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wduplicate-enum")
        AX_CHECK_COMPILE_FLAG(-Wempty-body,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wempty-body")
        AX_CHECK_COMPILE_FLAG(-Wenum-compare,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wenum-compare")
        AX_CHECK_COMPILE_FLAG(-Wextra,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wextra")
        AX_CHECK_COMPILE_FLAG(-Wformat-security,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-security")
        AX_CHECK_COMPILE_FLAG(-Wheader-guard,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wheader-guard")
        AX_CHECK_COMPILE_FLAG(-Wincompatible-pointer-types-discards-qualifiers, _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wincompatible-pointer-types-discards-qualifiers")
        AX_CHECK_COMPILE_FLAG(-Winit-self,                      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Winit-self")
        AX_CHECK_COMPILE_FLAG(-Wlogical-not-parentheses,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-not-parentheses")
        AX_CHECK_COMPILE_FLAG(-Wlogical-op-parentheses,         _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op-parentheses")
        AX_CHECK_COMPILE_FLAG(-Wloop-analysis,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wloop-analysis")
        AX_CHECK_COMPILE_FLAG(-Wuninitialized,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wuninitialized")
        AX_CHECK_COMPILE_FLAG(-Wno-missing-field-initializers,  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-missing-field-initializers")
        AX_CHECK_COMPILE_FLAG(-Wno-sign-compare,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-sign-compare")
        AX_CHECK_COMPILE_FLAG(-Wno-unused-const-variable,     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-const-variable")
        AX_CHECK_COMPILE_FLAG(-Wno-unused-parameter,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-parameter")
        AX_CHECK_COMPILE_FLAG(-Wno-variadic-macros,             _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-variadic-macros")
        AX_CHECK_COMPILE_FLAG(-Wparentheses,                    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wparentheses")
        AX_CHECK_COMPILE_FLAG(-Wpointer-bool-conversion,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wpointer-bool-conversion")
        AX_CHECK_COMPILE_FLAG(-Wsizeof-array-argument,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wsizeof-array-argument")
        AX_CHECK_COMPILE_FLAG(-Wwrite-strings,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wwrite-strings")
        AX_CHECK_COMPILE_FLAG(-fdiagnostics-show-option,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fdiagnostics-show-option")
        AX_CHECK_COMPILE_FLAG(-fno-omit-frame-pointer,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-omit-frame-pointer")
        AX_CHECK_COMPILE_FLAG(-fno-optimize-sibling-calls,      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-optimize-sibling-calls")
        AX_CHECK_COMPILE_FLAG(-fsanitize-address,               _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fsanitize-address")
        AX_CHECK_COMPILE_FLAG(-fstack-protector,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fstack-protector")

        EXTRA_CFLAGS="$_MAINTAINER_CFLAGS"
        CFLAGS="-g -O0 -Wall $CFLAGS"
        CXXFLAGS="-g -O0 -Wall $CXXFLAGS"
    fi

    PHP_ADD_LIBRARY(boost_filesystem, 1, YASD_SHARED_LIBADD)

    AS_CASE([$host_os],
      [darwin*], [YASD_OS="MAC"],
      [cygwin*], [YASD_OS="CYGWIN"],
      [mingw*], [YASD_OS="MINGW"],
      [linux*], [YASD_OS="LINUX"],
      []
    )

    PHP_SUBST(YASD_SHARED_LIBADD)

    PHP_ADD_INCLUDE([$ext_srcdir])
    PHP_ADD_INCLUDE([$ext_srcdir/include])

    yasd_source_file=" \
        yasd.cc \
        yasd_api.cc \
        php_yasd_cxx.cc \
        src/common.cc \
        src/util.cc \
        src/base.cc \
        src/buffer.cc \
        src/logger.cc \
        src/context.cc \
        src/global.cc \
        src/source_reader.cc \
        src/dbgp.cc \
        src/debuger_mode_base.cc \
        src/cmder_debugger.cc \
        src/remote_debugger.cc \
        src/base64.cc
    "

    yasd_source_file="$yasd_source_file \
        thirdparty/tinyxml2/tinyxml2.cc"

    PHP_NEW_EXTENSION(yasd, $yasd_source_file, $ext_shared,,$EXTRA_CFLAGS, cxx)

    # tinyxml2
    PHP_ADD_INCLUDE([$ext_builddir/thirdparty/tinyxml2])

    PHP_REQUIRE_CXX()

    CXXFLAGS="$CXXFLAGS -Wall -Wno-unused-function -Wno-deprecated -Wno-deprecated-declarations"

    if test "$YASD_OS" = "CYGWIN" || test "$YASD_OS" = "MINGW"; then
        CXXFLAGS="$CXXFLAGS -std=gnu++11"
    else
        CXXFLAGS="$CXXFLAGS -std=c++11"
    fi

    # tinyxml2
    PHP_ADD_BUILD_DIR($ext_builddir/thirdparty/tinyxml2)
fi
