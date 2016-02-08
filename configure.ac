#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

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
  "$SYSTEMOC_MAJOR_VERSION",
  [For SysteMoC version x.y.z defined to major version, e.g., x])
AC_DEFINE_UNQUOTED([SYSTEMOC_MINOR_VERSION],
  "$SYSTEMOC_MINOR_VERSION",
  [For SysteMoC version x.y.z defined to minor version, e.g., y])
AC_DEFINE_UNQUOTED([SYSTEMOC_MICRO_VERSION],
  "$SYSTEMOC_MICRO_VERSION",
  [For SysteMoC version x.y.z defined to micro version, e.g., z])

AC_CONFIG_SRCDIR([smoc-core/smoc_root_node.cpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

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
# Checks for programs.
#AC_PROG_LIBTOOL
#AC_PROG_INSTALL
#AC_PROG_LN_S
#AC_PROG_MAKE_SET

AC_DISABLE_SHARED
AC_PROG_LIBTOOL

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])
if test x"$enable_debug" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_DEBUG], [yes], [Defined if debug support is enabled])
fi

AC_CONFIG_HEADER([config.h pkginclude/systemoc/smoc_config.h])

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN("AC_PACKAGE_NAME", [doxygen.cfg], [doxygen-doc])
DX_ENV_APPEND(COSUPPORT_INCPATH, [\$(COSUPPORT_INCPATH)])
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
ACJF_CHECK_LIB_COSUPPORT_SMARTPTR([[version:>= 1.0]])
ACJF_CHECK_LIB_COSUPPORT_MATH([[version:>= 1.0]])

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
# This is a define for pkginclude/systemoc/smoc_config.h to guarantee
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
# This is a define for pkginclude/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_dataflow_trace" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_DATAFLOW_TRACE], [yes], [Defined if tracing support is enabled])
fi

### HOOKING ###
# --enable-hooking option
AC_ARG_ENABLE(
 [hooking],
 [[  --enable-hooking        Turn on hooking infrastructure [default=no]]],
 [case "${enableval}" in
    yes)  enable_hooking=yes  ;;
    no)   enable_hooking=no   ;;
    *)    AC_MSG_ERROR(bad value ${enableval} for --enable-hooking) ;;
  esac],
 [enable_hooking=no #default]
)

AM_CONDITIONAL([SYSTEMOC_ENABLE_HOOKING], [test x"$enable_hooking" = x"yes"])
# This is a define for pkginclude/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_hooking" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_HOOKING], [yes], [Defined if hooking infrastructure is provided])
fi

SYSTEMOC_OPTIONAL_REQUIRES=""
ACJF_CHECK_LIB_SGX([enable_sgx=yes], [enable_sgx=no])
dnl AM_CONDITIONAL([SYSTEMOC_ENABLE_SGX], [test x"$enable_sgx" = x"yes"])
# This is a define for pkginclude/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_sgx" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_SGX], [yes], [Defined if SGX support is enabled])
  SYSTEMOC_OPTIONAL_REQUIRES="${SYSTEMOC_OPTIONAL_REQUIRES} libsgx"
fi

ACJF_CHECK_LIB_SYSTEMC_VPC([enable_vpc=yes], [enable_vpc=no])
dnl AM_CONDITIONAL([SYSTEMOC_ENABLE_VPC], [test x"$enable_vpc" = x"yes"])
# This is a define for pkginclude/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_vpc" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_VPC], [yes], [Defined if SystemC-VPC support is enabled])
  SYSTEMOC_OPTIONAL_REQUIRES="${SYSTEMOC_OPTIONAL_REQUIRES} libsystemcvpc"
fi

ACJF_CHECK_LIB_MAESTRO_METAMAP([enable_maestro_metamap=yes], [enable_maestro_metamap=no])
AM_CONDITIONAL([SYSTEMOC_ENABLE_MAESTRO_METAMAP], [test x"$enable_maestro_metamap" = x"yes"])
# This is a define for pkginclude/systemoc/smoc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_maestro_metamap" = x"yes"; then
  AC_DEFINE([SYSTEMOC_ENABLE_MAESTRO_METAMAP], [yes], [Defined if Maestro MetaMap library support is enabled])
  SYSTEMOC_OPTIONAL_REQUIRES="${SYSTEMOC_OPTIONAL_REQUIRES} libmaestro-metamap >= 0.3"
fi
AC_SUBST([SYSTEMOC_OPTIONAL_REQUIRES])

AC_CONFIG_FILES([
  Makefile
  pkginclude/Makefile
  pkgconfig/libsystemoc.pc
])

ACJF_DONE

AC_OUTPUT
