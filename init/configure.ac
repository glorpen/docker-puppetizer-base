#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([puppetizer-init], [1.0.0], [arkadiusz.dziegiel@glorpen.pl])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
# AM_INIT_AUTOMAKE

: ${CFLAGS="-Os"}

# Checks for programs.
# AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[#include <argp.h>],
		[int argc=1; char *argv[]={"test"}; argp_parse(0,argc,argv,0,0,0); return 0;]
		)],
	[libc_has_argp="true"],
	[libc_has_argp="false"]
)

dnl If libc doesn't provide argp, then test for libargp
if test "$libc_has_argp" = "false" ; then
	AC_CHECK_LIB([argp], [argp_parse], [have_argp="true"], [have_argp="false"])

	if test "$have_argp" = "false"; then
		AC_MSG_ERROR("no libargp found")
	else
		LIBS="$LIBS -largp"
	fi
fi

AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR("no pthreads found")])

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h], [], [AC_MSG_ERROR("required header not found")])

# tests
AC_SUBST([TEST_LIBS])
AC_CHECK_HEADER(check.h)
AC_CHECK_LIB([check], [tcase_create], [TEST_LIBS="$TEST_LIBS -lcheck"])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strdup strerror], [], [AC_MSG_ERROR("required functions were not found")])

AC_ARG_WITH(puppetizer-service-dir,
	AS_HELP_STRING([--with-puppetizer-service-dir], [Path to service files]),
	[AC_DEFINE_UNQUOTED(PUPPETIZER_SERVICE_DIR, "$withval")],
	[AC_DEFINE(PUPPETIZER_SERVICE_DIR, "/opt/puppetizer/etc/services")]
)
AC_ARG_WITH(puppetizer-apply,
	AS_HELP_STRING([--with-puppetizer-apply], [Path to puppetizer apply script]),
	[AC_DEFINE_UNQUOTED(PUPPETIZER_APPLY, "$withval")],
	[AC_DEFINE(PUPPETIZER_APPLY, "/opt/puppetizer/bin/apply")]
)
AC_ARG_WITH(puppetizer-control-socket,
	AS_HELP_STRING([--with-control-socket], [Path to init control socket]),
	[AC_DEFINE_UNQUOTED(PUPPETIZER_CONTROL_SOCKET, "$withval")],
	[AC_DEFINE(PUPPETIZER_CONTROL_SOCKET, "/opt/puppetizer/run/control.socket")]
)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
