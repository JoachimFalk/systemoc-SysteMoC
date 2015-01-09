#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(
  [SysteMoC-Tutorial Mainline Branch], [0.1],
  [falk@cs.fau.de],
  [SysteMoC-Tutorial--mainline-])

AC_PREREQ(2.57)

AC_PATH_PROGS(BASH, bash)
AC_PATH_PROGS(PERL, perl)

ACJF_INIT

AC_CONFIG_SRCDIR([source-sink.cpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_LANG([C++])

dnl Checks for programs.
ACJF_CHECK_CROSSTOOL(CC, gcc)
ACJF_CHECK_CROSSTOOL(CXX, g++)
ACJF_CHECK_CROSSTOOL(AR, ar)
ACJF_CHECK_CROSSTOOL(RANLIB, ranlib)
ACJF_CHECK_CROSSTOOL(NM, nm)
AC_PROG_CXXCPP
AC_PROG_CPP
dnl AC_PROG_LIBTOOL
dnl AC_PROG_INSTALL
dnl AC_PROG_LN_S
dnl AC_PROG_MAKE_SET

AC_DISABLE_SHARED
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE([subdir-objects])

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])

ACJF_CHECK_LIB_SYSTEMC
dnl AM_CPPFLAGS="$AM_CPPFLAGS $SYSTEMC_INCLUDE"
dnl AM_LDFLAGS="$AM_LDFLAGS   $SYSTEMC_LDFLAGS"
ACJF_CHECK_LIB_BOOST
dnl AM_CPPFLAGS="$AM_CPPFLAGS $BOOST_INCLUDE"
dnl AM_LDFLAGS="$AM_LDFLAGS   $BOOST_LDFLAGS"
ACJF_CHECK_LIB_COSUPPORT
ACJF_CHECK_LIB_SYSTEMOC

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

dnl check for exe extension
AC_EXEEXT

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

SUBDIRS=$subdirs
AC_SUBST(SUBDIRS)

ACJF_DONE

AC_OUTPUT
