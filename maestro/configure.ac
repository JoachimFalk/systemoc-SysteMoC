#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(
  [Testcases maestro Branch], [1.0],
  [rafael.rosales@fau.de],
  [testcases-top--devel-])

AC_PREREQ(2.57)


AC_PATH_PROGS(BASH, bash)
AC_PATH_PROGS(PERL, perl)

ACJF_INIT

AC_CONFIG_AUX_DIR([../../BuildSystem])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_LANG([C++])

dnl Checks for programs.
ACJF_CHECK_CROSSTOOL(CC, gcc)
ACJF_CHECK_CROSSTOOL(CXX, g++)
ACJF_CHECK_CROSSTOOL(AR, ar)
ACJF_CHECK_CROSSTOOL(RANLIB, ranlib)
ACJF_CHECK_CROSSTOOL(NM, nm)

ACJF_ENABLE_DIALECTS_C99_CXX11

AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CXXCPP
AC_PROG_CPP
dnl AC_PROG_LIBTOOL
dnl AC_PROG_INSTALL
dnl AC_PROG_LN_S
dnl AC_PROG_MAKE_SET

AC_DISABLE_SHARED
AC_PROG_LIBTOOL

dnl check for exe extension
AC_EXEEXT

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])

ACJF_CHECK_LIB_SYSTEMC
ACJF_CHECK_LIB_BOOST
ACJF_CHECK_LIB_COSUPPORT
ACJF_CHECK_LIB_SYSTEMOC

if test x"$SYSTEMOC_ENABLE_MAESTRO" != x"yes"; then
  AC_MSG_ERROR([No Maestro support in SysteMoC library, bailing out!])
fi

dnl ACJF_CHECK_LIB_MAESTRO_METAMAP

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

dnl m4_sinclude([automotive-testcase/configure.in.frag])
dnl m4_sinclude([src-snk/configure.in.frag])
dnl m4_sinclude([vpc-api/configure.in.frag])
dnl m4_sinclude([automotive-energy/configure.in.frag])
dnl m4_sinclude([automotive-diagnosis/configure.in.frag])
dnl m4_sinclude([dynamic-dse/configure.in.frag])

SUBDIRS=$subdirs
AC_SUBST(SUBDIRS)

ACJF_DONE

AC_OUTPUT
