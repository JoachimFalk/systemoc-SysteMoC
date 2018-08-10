dnl Process this file with autoconf to produce a configure script.
dnl
dnl vim: set sw=2 ts=8:
dnl
dnl Copyright (C) Joachim Falk <joachim.falk@gmx.de> $Date: 2002/05/18 20:20:33 $
dnl
dnl configure.in is part of the jf_libs distribution of Joachim Falk;
dnl you can redistribute it and/or modify it under the terms of the
dnl GNU General Public License as published by the Free Software Foundation;
dnl either version 2 of the License, or (at your option) any later version.
dnl
dnl The jf_libs distributio is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public
dnl License along with this program; If not, write to
dnl the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.

AC_INIT(
  [ActorLibrary Mainline Branch], [0.1],
  [liyuan.zhang@informatik.uni-erlangen.de],
  [ActorLibrary--mainline-])

AC_PREREQ(2.57)
ACJF_INIT

AC_CONFIG_SRCDIR([src/actorlibrary/headers/actorlibrary/sources/generic/PeriodicTask.hpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Checks for programs.
ACJF_CHECK_CROSSTOOL([CC], [gcc])
ACJF_CHECK_CROSSTOOL([CXX], [c++])
ACJF_CHECK_CROSSTOOL([AR], [ar])
ACJF_CHECK_CROSSTOOL([RANLIB], [ranlib])
ACJF_CHECK_CROSSTOOL([NM], [nm])

ACJF_ENABLE_DIALECTS_C99_CXX11

AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CXXCPP
AC_PROG_CPP

AC_DISABLE_SHARED
AC_PROG_LIBTOOL

AC_CONFIG_HEADER([src/actorlibrary/cpp/config.h])

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])

dnl check for exe extension
AC_EXEEXT

dnl Checks for libraries.
ACJF_CHECK_LIB_SYSTEMOC

dnl Checks for header files.
#AC_HEADER_DIRENT
#AC_HEADER_STDC
#AC_CHECK_HEADERS(fcntl.h limits.h syslog.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl which subdirs do I have

SUBDIRS="$subdirs"
AC_SUBST(SUBDIRS)

AC_CONFIG_FILES([
  Makefile
  src/actorlibrary/headers/Makefile
  pkgconfig/libactorlibrary.pc
])

ACJF_DONE

AC_OUTPUT
