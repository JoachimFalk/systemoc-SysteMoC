dnl Checks for header files.
dnl AC_HEADER_DIRENT
dnl AC_HEADER_STDC
dnl AC_CHECK_HEADERS(fcntl.h limits.h syslog.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.

ACJF_CHECK_PKG([ActorLibrary])

AC_CONFIG_FILES([
  automotive-testcase/Makefile
])

subdirs="$subdirs automotive-testcase"
