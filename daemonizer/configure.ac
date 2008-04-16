AC_INIT(daemonizer, 0.9.0, matteo.corti@gmail.com)
AM_INIT_AUTOMAKE(daemonizer, 0.9.0, matteo.corti@gmail.com)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AM_PROG_CC_C_O
AC_ISC_POSIX

AC_STDC_HEADERS
AC_HAVE_HEADERS(errno.h fcntl.h getopt.h libgen.h signal.h stdint.h stdio.h stdlib.h string.h strings.h sys/socket.h sys/stat.h sys/types.h sys/wait.h time.h unistd.h)

AC_CHECK_FUNCS(clearenv)

dnl AC_CHECK_LIB(m, ceil,,[AC_MSG_ERROR([libm not found])])

dnl this macro is used to get the arguments supplied
dnl to the configure script (./configure --enable-debug)
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging [default=yes]],, enable_debug=$debug_default)
case "${enable_debug}" in
  yes) CFLAGS="$CFLAGS -g -DDEBUG"; AC_MSG_RESULT(yes) ;;
  no)  AC_MSG_RESULT(no);;
  *) AC_MSG_ERROR(bad value ${enable_debug} for --enable-debug) ;;
esac

AC_OUTPUT(daemonizer.1 VERSION daemonizer.spec)
