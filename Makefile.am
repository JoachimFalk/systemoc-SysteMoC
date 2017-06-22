## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (C) Joachim Falk <joachim.falk@gmx.de> $Date: 2003/01/15 16:43:20 $
#
# Makefile.am is part of the jflib distribution of Joachim Falk;
# you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation;
# either version 2 of the License, or (at your option) any later version.
#
# The jflib distributio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# $Log$

include $(auxdir)/jfrules.mk
include ../BuildSystem/doxygen.am

SUBDIRS = src/systemoc/headers

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

AM_CPPFLAGS= @AM_CPPFLAGS@ \
	-Isrc/systemoc/headers \
	-I$(srcdir)/src/systemoc/headers

AM_LDFLAGS = @AM_LDFLAGS@

pkgconfigdir = $(libdir)/pkgconfig

nodist_pkgconfig_DATA =	\
	libsystemoc.pc

%.pc: pkgconfig/%.pc
	@sed -e '/^srctreemode_\w\+\s*=/d' \
	     -e 's/\$${srctreemode_libdir}\/lib\([^ 	]*\).la/-l\1/g' \
	     -e 's/\$${srctreemode_\(\w\+\)}/\$${\1}/g' \
	  < $^ > $@

CLEANFILES=$(nodist_pkgconfig_DATA)

lib_LTLIBRARIES = libsystemoc.la

libsystemoc_la_SOURCES = \
	src/systemoc/cpp/DebugOStream.cpp \
	src/systemoc/cpp/GraphBase.cpp \
	src/systemoc/cpp/IdPool.cpp \
	src/systemoc/cpp/IOPattern.cpp \
	src/systemoc/cpp/LatencyQueue.cpp \
	src/systemoc/cpp/Node.cpp \
	src/systemoc/cpp/NodeQueue.cpp \
	src/systemoc/cpp/SimulationContext.cpp \
	src/systemoc/cpp/smoc_actor.cpp \
	src/systemoc/cpp/smoc_chan_if.cpp \
	src/systemoc/cpp/smoc_elab_and_sim.cpp \
	src/systemoc/cpp/smoc_expr.cpp \
	src/systemoc/cpp/smoc_fifo.cpp \
	src/systemoc/cpp/smoc_firing_rules.cpp \
	src/systemoc/cpp/smoc_func_call.cpp \
	src/systemoc/cpp/smoc_graph.cpp \
	src/systemoc/cpp/smoc_hooking.cpp \
	src/systemoc/cpp/smoc_main.cpp \
	src/systemoc/cpp/smoc_multicast_sr_signal.cpp \
	src/systemoc/cpp/smoc_multiplex_fifo.cpp \
	src/systemoc/cpp/smoc_multireader_fifo.cpp \
	src/systemoc/cpp/smoc_periodic_actor.cpp \
	src/systemoc/cpp/smoc_reset.cpp \
	src/systemoc/cpp/smoc_root_chan.cpp \
	src/systemoc/cpp/smoc_scheduler_top.cpp \
	src/systemoc/cpp/smoc_sysc_port.cpp \
	src/systemoc/cpp/SMXDumper.cpp \
	src/systemoc/cpp/SysteMoCScheduler.cpp \
	src/systemoc/cpp/TraceLog.cpp \
	src/systemoc/cpp/VpcInterface.cpp

libsystemoc_la_CPPFLAGS = $(AM_CPPFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(SYSTEMC_CPPFLAGS) \
	$(TLM1_CPPFLAGS) \
	$(COSUPPORT_SYSTEMC_CPPFLAGS) \
	$(COSUPPORT_TRACING_CPPFLAGS) \
	$(COSUPPORT_STREAMS_CPPFLAGS) \
	$(COSUPPORT_SMARTPTR_CPPFLAGS) \
	$(COSUPPORT_ALLOCATORS_CPPFLAGS) \
	$(COSUPPORT_MATH_CPPFLAGS) \
	$(COSUPPORT_STRING_CPPFLAGS) \
	$(LIBSGX_CPPFLAGS) \
	$(SYSTEMC_VPC_CPPFLAGS) \
	$(MAESTRO_METAMAP_CPPFLAGS)
libsystemoc_la_LDFLAGS = $(AM_LDFLAGS) \
	$(BOOST_LDFLAGS) \
	$(SYSTEMC_LDFLAGS) \
	$(TLM1_LDFLAGS) \
	$(COSUPPORT_SYSTEMC_LDFLAGS) \
	$(COSUPPORT_TRACING_LDFLAGS) \
	$(COSUPPORT_STREAMS_LDFLAGS) \
	$(COSUPPORT_SMARTPTR_LDFLAGS) \
	$(COSUPPORT_ALLOCATORS_LDFLAGS) \
	$(COSUPPORT_MATH_LDFLAGS) \
	$(COSUPPORT_STRING_LDFLAGS) \
	$(LIBSGX_LDFLAGS) \
	$(SYSTEMC_VPC_LDFLAGS) \
	$(MAESTRO_METAMAP_LDFLAGS) \
	-version-info 0:0:0 \
	-no-undefined
libsystemoc_la_LIBADD = \
	-lboost_program_options$(BOOST_LIBPOSTFIX) \
	-lsystemc \
	$(COSUPPORT_SYSTEMC_LIBS) \
	$(COSUPPORT_TRACING_LIBS) \
	$(COSUPPORT_STREAMS_LIBS) \
	$(COSUPPORT_SMARTPTR_LIBS) \
	$(COSUPPORT_ALLOCATORS_LIBS) \
	$(COSUPPORT_MATH_LIBS) \
	$(COSUPPORT_STRING_LIBS) \
	$(LIBSGX_LIBS) \
	$(SYSTEMC_VPC_LIBS) \
	$(MAESTRO_METAMAP_LIBS)
libsystemoc_la_DEPENDENCIES = \
	$(COSUPPORT_SYSTEMC_DEPENDENCIES) \
	$(COSUPPORT_TRACING_DEPENDENCIES) \
	$(COSUPPORT_STREAMS_DEPENDENCIES) \
	$(COSUPPORT_SMARTPTR_DEPENDENCIES) \
	$(COSUPPORT_ALLOCATORS_DEPENDENCIES) \
	$(COSUPPORT_MATH_DEPENDENCIES) \
	$(COSUPPORT_STRING_DEPENDENCIES) \
	$(LIBSGX_DEPENDENCIES) \
	$(SYSTEMC_VPC_DEPENDENCIES) \
	$(MAESTRO_METAMAP_DEPENDENCIES)
