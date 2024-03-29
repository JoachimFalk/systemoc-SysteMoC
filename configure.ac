# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2010 FAU -- Christian Zebelein <christian.zebelein@fau.de>
#   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2011 Christian Zebelein <christian.zebelein@uni-rostock.de>
#   2011 FAU -- Christian Zebelein <christian.zebelein@fau.de>
#   2011 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
#   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2012 Rafael Rosales <rafael.rosales@informatik.uni-erlangen.de>
#   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2016 Adnane Tayachi <adnane.tayachi@gmx.de>
#   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Martin Letras <martin.letras@fau.de>
#   2017 FAU -- Simone Müller <simone.mueller@fau.de>
#   2018 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
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
  [SysteMoC Devel Branch], [1.0],
  [joachim.falk@informatik.uni-erlangen.de],
  [SysteMoC--devel-])
AC_PREREQ(2.57)
ACJF_INIT

SYSTEMOC_MAJOR_VERSION=
SYSTEMOC_MINOR_VERSION=
SYSTEMOC_MICRO_VERSION=
SYSTEMOC_VERSION="$PACKAGE_VERSION"
eval `echo $PACKAGE_VERSION | sed -ne ['/^[0-9][0-9]*\(\.[0-9][0-9]*\|\)\(\.[0-9][0-9]*\|\)$/{s/^\([0-9][0-9]*\)\(\.\([0-9][0-9]*\)\|\)\(\.\([0-9][0-9]*\)\|\).*$/SYSTEMOC_MAJOR_VERSION=\1; SYSTEMOC_MINOR_VERSION=\3; SYSTEMOC_MICRO_VERSION=\5;/;p}']`
test x"$SYSTEMOC_MAJOR_VERSION" = x"" && SYSTEMOC_MAJOR_VERSION=0
test x"$SYSTEMOC_MINOR_VERSION" = x"" && SYSTEMOC_MINOR_VERSION=0
test x"$SYSTEMOC_MICRO_VERSION" = x"" && SYSTEMOC_MICRO_VERSION=0
dnl echo "\$SYSTEMOC_MAJOR_VERSION: $SYSTEMOC_MAJOR_VERSION"
dnl echo "\$SYSTEMOC_MINOR_VERSION: $SYSTEMOC_MINOR_VERSION"
dnl echo "\$SYSTEMOC_MICRO_VERSION: $SYSTEMOC_MICRO_VERSION"
AC_DEFINE_UNQUOTED([SYSTEMOC_VERSION],
  "$SYSTEMOC_VERSION",
  [Defined to SysteMoC version])
AC_DEFINE_UNQUOTED([SYSTEMOC_MAJOR_VERSION],
  $SYSTEMOC_MAJOR_VERSION,
  [For SysteMoC version x.y.z defined to major version, e.g., x])
AC_DEFINE_UNQUOTED([SYSTEMOC_MINOR_VERSION],
  $SYSTEMOC_MINOR_VERSION,
  [For SysteMoC version x.y.z defined to minor version, e.g., y])
AC_DEFINE_UNQUOTED([SYSTEMOC_MICRO_VERSION],
  $SYSTEMOC_MICRO_VERSION,
  [For SysteMoC version x.y.z defined to micro version, e.g., z])

AC_CONFIG_SRCDIR([src/systemoc/cpp/smoc_actor.cpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

dnl AC_CANONICAL_BUILD
dnl AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CXX

ACJF_ENABLE_DIALECTS_C99_CXX11

AM_INIT_AUTOMAKE([subdir-objects])

AC_DISABLE_SHARED
LT_INIT

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])
if test x"$enable_debug" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_DEBUG], [yes], [Defined if debug support is enabled])
fi

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN("AC_PACKAGE_NAME", [doxygen.cfg], [doxygen-doc])
DX_ENV_APPEND(COSUPPORT_INCPATH, [\$(COSUPPORT_STRING_INCPATH)])
COSUPPORT_EXTRA='`find $(COSUPPORT_STRING_INCPATH) -name "transform.hpp" -o -name "*Facade.hpp" -o -name "List*.hpp" -o -name "Set*.hpp" -o -name "Vector*.hpp" | tr "\n" " "`'
AC_SUBST([COSUPPORT_EXTRA])
DX_ENV_APPEND(COSUPPORT_EXTRA, ['\$(COSUPPORT_EXTRA)'])
DX_ENV_APPEND(BOOST_INCPATH, [\$(BOOST_INCPATH)])


dnl check for exe extension
AC_EXEEXT

# Checks for header files.
# AC_FUNC_ALLOCA
# AC_HEADER_STDC
# AC_CHECK_HEADERS([OS.h fcntl.h limits.h mach/mach.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

ACJF_CHECK_LIB_SYSTEMC
ACJF_CHECK_LIB_TLM1
ACJF_CHECK_LIB_BOOST
ACJF_CHECK_LIB_COSUPPORT_SYSTEMC([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_TRACING([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_STREAMS([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_RANDOM([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_SMARTPTR([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_MATH([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_ALLOCATORS([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_STRING([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_XERCES([[version:>= 1.0]])

AC_CHECK_LIB([ltdl], [lt_dlinit],
  [],
  [AC_MSG_ERROR([Cannot find libltdl library, bailing out!])])

### TRACING ###
# --enable-transition-trace option
AC_ARG_ENABLE(
 [transition-trace],
 [[  --enable-transition-trace Turn on transition tracing support [default=yes]]],
 [case "${enableval}" in
    yes)  enable_transition_trace=yes  ;;
    no)   enable_transition_trace=no   ;;
    *)    AC_MSG_ERROR(bad value ${enableval} for --enable-transition-trace) ;;
  esac],
 [enable_transition_trace=yes #default]
)

AM_CONDITIONAL([SYSTEMOC_ENABLE_TRANSITION_TRACE], [test x"$enable_transition_trace" = x"yes"])
# This is a define for src/systemoc/headers/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_transition_trace" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_TRANSITION_TRACE], [yes], [Defined if tracing support is enabled])
fi

# --enable-dataflow-trace option
AC_ARG_ENABLE(
 [dataflow-trace],
 [[  --enable-dataflow-trace Turn on systemoc tracing [default=no]]],
 [case "${enableval}" in
    yes)  enable_dataflow_trace=yes  ;;
    no)   enable_dataflow_trace=no   ;;
    *)    AC_MSG_ERROR(bad value ${enableval} for --enable-dataflow-trace) ;;
  esac],
 [enable_dataflow_trace=no #default]
)

AM_CONDITIONAL([SYSTEMOC_ENABLE_DATAFLOW_TRACE], [test x"$enable_dataflow_trace" = x"yes"])
# This is a define for src/systemoc/headers/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_dataflow_trace" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_DATAFLOW_TRACE], [yes], [Defined if tracing support is enabled])
fi

### ROUTING SUPPORT ###
# --enable-routing option
AC_ARG_ENABLE(
 [routing],
 [[  --enable-routing Turn on support for simulator back ends to delay communication due to routing delays [default=yes]]],
 [case "${enableval}" in
    yes)  enable_routing=yes  ;;
    no)   enable_routing=no   ;;
    *)    AC_MSG_ERROR(bad value ${enableval} for --enable-routing) ;;
  esac],
 [enable_routing=yes #default]
)

AM_CONDITIONAL([SYSTEMOC_ENABLE_ROUTING], [test x"$enable_routing" = x"yes"])
# This is a define for src/systemoc/headers/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_routing" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_ROUTING], [yes], [Defined if routing support is enabled])
fi

### HOOKING ###
# --enable-hooking option
AC_ARG_ENABLE(
 [hooking],
 [[  --enable-hooking        Turn on hooking infrastructure [default=yes]]],
 [case "${enableval}" in
    yes)  enable_hooking=yes  ;;
    no)   enable_hooking=no   ;;
    *)    AC_MSG_ERROR(bad value ${enableval} for --enable-hooking) ;;
  esac],
 [enable_hooking=yes #default]
)

AM_CONDITIONAL([SYSTEMOC_ENABLE_HOOKING], [test x"$enable_hooking" = x"yes"])
# This is a define for src/systemoc/headers/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_hooking" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_HOOKING], [yes], [Defined if hooking infrastructure is provided])
fi

SYSTEMOC_OPTIONAL_REQUIRES=""
SYSTEMOC_OPTIONAL_REQUIRES_PRIVATE=""

ACJF_CHECK_LIB_SGX([enable_sgx=yes], [enable_sgx=no])
if test x"$enable_sgx" = x"yes"; then
  ACJF_CHECK_LIB_SGXUTILS([enable_sgxutils=yes], [enable_sgxutils=no])
  if test x"$enable_sgxutils" != x"yes"; then
    AC_MSG_ERROR([If libsgx is enable, we also require libsgxutils. However libsgxutils is missing, bailing out!])
  fi
  AC_DEFINE([SYSTEMOC_ENABLE_SGX], [yes], [Defined if SGX support is enabled])
  SYSTEMOC_OPTIONAL_REQUIRES_PRIVATE="${SYSTEMOC_OPTIONAL_REQUIRES_PRIVATE} libsgx libsgxutils"
fi
AM_CONDITIONAL([SYSTEMOC_ENABLE_SGX], [test x"$enable_sgx" = x"yes" -a x"$enable_sgxutils" = x"yes"])

ACJF_CHECK_LIB_NGANALYSIS([enable_nganalysis=yes], [enable_nganalysis=no])
if test x"$enable_nganalysis" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_NGANALYSIS], [yes], [Defined if network graph analysis support is enabled])
fi
AM_CONDITIONAL([SYSTEMOC_ENABLE_NGANALYSIS], [test x"$enable_nganalysis" = x"yes"])

AC_SUBST([SYSTEMOC_OPTIONAL_REQUIRES])
AC_SUBST([SYSTEMOC_OPTIONAL_REQUIRES_PRIVATE])

if test x"$enable_sgx" = x"yes"; then
  ACJF_PROG_SGXUTILS([sgxutils=true], [sgxutils=false])
else
  sgxutils=false
fi
AM_CONDITIONAL(SGXUTILS, test x"$sgxutils" = x"true")

AC_CONFIG_HEADERS(
 [config.h]dnl
 [src/systemoc/headers/systemoc/smoc_config.h]dnl
 [src/actorlibrary/cpp/config.h])
AC_CONFIG_FILES([
  Makefile
  src/systemoc-sng/headers/Makefile
  src/systemoc/headers/Makefile
  src/actorlibrary/headers/Makefile
  pkgconfig/libsystemoc.pc
  pkgconfig/libactorlibrary.pc
  testcases/Makefile
  tutorial/Makefile
  tools/Makefile
])

ACJF_DONE

AC_OUTPUT
