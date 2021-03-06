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
  [SysteMoC Testcases Devel Branch], [1.0],
  [falk@cs.fau.de],
  [SysteMoC-Testcases--devel-])

AC_PREREQ(2.57)

AC_PATH_PROGS(BASH, bash)
AC_PATH_PROGS(PERL, perl)

ACJF_INIT

AC_CONFIG_SRCDIR([Testcases.txt])
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

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])

ACJF_CHECK_LIB_SYSTEMC
dnl AM_CPPFLAGS="$AM_CPPFLAGS $SYSTEMC_INCLUDE"
dnl AM_LDFLAGS="$AM_LDFLAGS   $SYSTEMC_LDFLAGS"
ACJF_CHECK_LIB_BOOST
dnl AM_CPPFLAGS="$AM_CPPFLAGS $BOOST_INCLUDE"
dnl AM_LDFLAGS="$AM_LDFLAGS   $BOOST_LDFLAGS"
ACJF_CHECK_LIB_COSUPPORT
ACJF_CHECK_LIB_SYSTEMOC
ACJF_CHECK_LIB_SYSTEMC_VPC([], [false])

SYSTEMOC_PLUGINPATH=""

AM_CONDITIONAL(VPC, test x"$SYSTEMC_VPC_FOUND" = x"yes")
if test x"$SYSTEMC_VPC_FOUND" = x"yes"; then
  AC_CONFIG_SUBDIRS([vpc])
  for dir in ${SYSTEMC_VPC_LIBPATH}; do
    if test -d $dir/.libs; then
      dir=$dir/.libs
    fi
    if test x"$SYSTEMOC_PLUGINPATH" = x""; then
      SYSTEMOC_PLUGINPATH="$dir";
    else
      SYSTEMOC_PLUGINPATH="$SYSTEMOC_PLUGINPATH:$dir";
    fi
  done
fi
AM_CONDITIONAL(MAESTRO, test x"$SYSTEMOC_ENABLE_MAESTRO" = x"yes")
if test x"$SYSTEMOC_ENABLE_MAESTRO" = x"yes"; then
  AC_CONFIG_SUBDIRS([maestro])
fi
AC_SUBST([SYSTEMOC_PLUGINPATH])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

dnl check for exe extension
AC_EXEEXT

ACJF_CHECK_PKG([Synthesis-SW], [Synthesis-SW], [synth_sw=true], [synth_sw=false])
AM_CONDITIONAL(SYNTH_SW, test x"$synth_sw" = x"true")
ACJF_CHECK_PKG([Synthesis-Pthread], [Synthesis-Pthread], [synth_pthread=true], [synth_pthread=false])
AM_CONDITIONAL(SYNTH_PTHREAD, test x"$synth_pthread" = x"true")
ACJF_CHECK_PKG([Synthesis-VivadoHLS], [Synthesis-VivadoHLS], [synth_vivadohls=true], [synth_vivadohls=false])
AM_CONDITIONAL(SYNTH_VIVADOHLS, test x"$synth_vivadohls" = x"true")
ACJF_CHECK_PKG([Synthesis-TLM], [Synthesis-TLM], [synth_tlm=true], [synth_tlm=false])
AM_CONDITIONAL(SYNTH_TLM, test x"$synth_tlm" = x"true")
ACJF_CHECK_PKG([Clustering], [Clustering], [clustering=true], [clustering=false])
AM_CONDITIONAL(CLUSTERING, test x"$clustering" = x"true")

ACJF_PROG_SGXUTILS([sgxutils=true], [sgxutils=false])
AM_CONDITIONAL(SGXUTILS, test x"$sgxutils" = x"true")

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])


dnl m4_sinclude([systemoc-sqr/configure.in.frag])
dnl m4_sinclude([systemoc-sqr-int/configure.in.frag])
dnl m4_sinclude([systemoc-cluster-example/configure.in.frag])
dnl m4_sinclude([systemoc-fir/configure.in.frag])
dnl m4_sinclude([systemoc-getValueAt/configure.in.frag])
dnl m4_sinclude([systemoc-sdf/configure.in.frag])
dnl m4_sinclude([systemoc-select/configure.in.frag])
dnl m4_sinclude([systemoc-event/configure.in.frag])
dnl m4_sinclude([systemoc-periodic/configure.in.frag])
dnl m4_sinclude([systemoc-fsm-and-composition/configure.in.frag])
dnl m4_sinclude([systemoc-fsm-xor-composition/configure.in.frag])
dnl m4_sinclude([systemoc-fsm-junction-state/configure.in.frag])
dnl m4_sinclude([systemoc-fsm-hierarchical/configure.in.frag])
dnl m4_sinclude([systemoc-dynamic-fsm/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-hierarchy/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-multicast/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-multiplex/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-multireader/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-multireader-sched/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-multiwriter/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-noconsume/configure.in.frag])
dnl m4_sinclude([systemoc-src-snk-void/configure.in.frag])
dnl m4_sinclude([systemoc-systemc/configure.in.frag])

SUBDIRS=$subdirs
AC_SUBST(SUBDIRS)

ACJF_DONE

AC_OUTPUT
