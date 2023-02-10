# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2023 FAU -- Joachim Falk <joachim.falk@fau.de>
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
# 
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

##                                               -*- Autoconf -*-
## Process this file with autoconf to produce a configure script.

AC_INIT(
  [ActorLibrary Mainline Branch], [0.1],
  [joachim.falk@fau.de],
  [ActorLibrary--mainline-])
AC_PREREQ(2.57)
ACJF_INIT

AC_CONFIG_SRCDIR([src/actorlibrary/headers/actorlibrary/sources/generic/PeriodicTask.hpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

dnl AC_CANONICAL_BUILD
dnl AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXXCPP
AC_PROG_CPP

ACJF_ENABLE_DIALECTS_C99_CXX11

AM_INIT_AUTOMAKE([subdir-objects])

AC_DISABLE_SHARED
LT_INIT

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])

# Check for exe extension
AC_EXEEXT

# Checks for header files.
#AC_HEADER_DIRENT
#AC_HEADER_STDC
#AC_CHECK_HEADERS(fcntl.h limits.h syslog.h unistd.h)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for libraries.
ACJF_CHECK_LIB_SYSTEMOC

AC_CONFIG_HEADERS(dnl
 [src/actorlibrary/cpp/config.h])
AC_CONFIG_FILES([
  Makefile
  src/actorlibrary/headers/Makefile
  pkgconfig/libactorlibrary.pc
])

ACJF_DONE

AC_OUTPUT
