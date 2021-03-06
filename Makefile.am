# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
#   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
#   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2016 Adnane Tayachi <adnane.tayachi@gmx.de>
#   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
#   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
#   2017 FAU -- Simone Müller <simone.mueller@fau.de>
#   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
#   2018 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

## Process this file with automake to produce Makefile.in

include $(auxdir)/jfrules.mk
include ../BuildSystem/doxygen.am

SUBDIRS = . src/systemoc/headers testcases tutorial

AUTOMAKE_OPTIONS = nostdinc
# This is only needed if there is a configure.ac in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

AM_CPPFLAGS= @AM_CPPFLAGS@ \
	-Isrc/systemoc/headers \
	-I$(srcdir)/src/systemoc/headers \
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
	$(LIBSGXUTILS_CPPFLAGS) \
	$(MAESTRO_METAMAP_CPPFLAGS)

AM_LDFLAGS = @AM_LDFLAGS@ \
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
	$(LIBSGXUTILS_LDFLAGS) \
	$(MAESTRO_METAMAP_LDFLAGS)

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
	src/systemoc/cpp/detail/ChanBase.cpp \
	src/systemoc/cpp/detail/DebugOStream.cpp \
	src/systemoc/cpp/detail/FSM/ANDStateImpl.cpp \
	src/systemoc/cpp/detail/FSM/BaseStateImpl.cpp \
	src/systemoc/cpp/detail/FSM/ExpandedTransition.cpp \
	src/systemoc/cpp/detail/FSM/FiringFSM.cpp \
        src/systemoc/cpp/detail/FSM/FiringStateImpl.cpp \
        src/systemoc/cpp/detail/FSM/JunctionStateImpl.cpp \
        src/systemoc/cpp/detail/FSM/MultiStateImpl.cpp \
	src/systemoc/cpp/detail/FSM/PartialTransition.cpp \
	src/systemoc/cpp/detail/FSM/RuntimeFiringRule.cpp \
	src/systemoc/cpp/detail/FSM/RuntimeState.cpp \
	src/systemoc/cpp/detail/FSM/RuntimeTransition.cpp \
	src/systemoc/cpp/detail/FSM/RuntimeTransitionHook.cpp \
        src/systemoc/cpp/detail/FSM/StateImpl.cpp \
        src/systemoc/cpp/detail/FSM/XORStateImpl.cpp \
	src/systemoc/cpp/detail/FifoChanBase.cpp \
	src/systemoc/cpp/detail/GraphBase.cpp \
	src/systemoc/cpp/detail/IdPool.cpp \
	src/systemoc/cpp/detail/NodeBase.cpp \
	src/systemoc/cpp/detail/NodeQueue.cpp \
	src/systemoc/cpp/detail/PortBase.cpp \
	src/systemoc/cpp/detail/RegisterChanBase.cpp \
	src/systemoc/cpp/detail/RegisterEntry.cpp \
	src/systemoc/cpp/detail/RegisterOutlet.cpp \
	src/systemoc/cpp/detail/SimulationContext.cpp \
	src/systemoc/cpp/detail/SMXDumper.cpp \
	src/systemoc/cpp/detail/SMXImporter.cpp \
	src/systemoc/cpp/detail/SNGDumper.cpp \
	src/systemoc/cpp/detail/SysteMoCSimulator.cpp \
	src/systemoc/cpp/detail/TraceLog.cpp \
	src/systemoc/cpp/smoc_action.cpp \
	src/systemoc/cpp/smoc_actor.cpp \
	src/systemoc/cpp/smoc_and_state.cpp \
	src/systemoc/cpp/smoc_base_state.cpp \
	src/systemoc/cpp/smoc_chan_if.cpp \
	src/systemoc/cpp/smoc_elab_and_sim.cpp \
	src/systemoc/cpp/smoc_expr.cpp \
	src/systemoc/cpp/smoc_firing_state.cpp \
	src/systemoc/cpp/smoc_graph.cpp \
	src/systemoc/cpp/smoc_junction_state.cpp \
	src/systemoc/cpp/smoc_main.cpp \
	src/systemoc/cpp/smoc_multiplex_fifo.cpp \
	src/systemoc/cpp/smoc_multireader_fifo.cpp \
	src/systemoc/cpp/smoc_multi_state.cpp \
	src/systemoc/cpp/smoc_periodic_actor.cpp \
	src/systemoc/cpp/smoc_port_out.cpp \
	src/systemoc/cpp/smoc_reset.cpp \
	src/systemoc/cpp/smoc_scheduler_top.cpp \
	src/systemoc/cpp/smoc_state.cpp \
	src/systemoc/cpp/smoc_xor_state.cpp

libsystemoc_la_CPPFLAGS = $(AM_CPPFLAGS)
libsystemoc_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info 0:0:0 \
	-Wl,--no-undefined
libsystemoc_la_LIBADD = \
	-lboost_program_options$(BOOST_LIBPOSTFIX) \
	-lsystemc \
	-lltdl \
	$(COSUPPORT_SYSTEMC_LIBS) \
	$(COSUPPORT_TRACING_LIBS) \
	$(COSUPPORT_STREAMS_LIBS) \
	$(COSUPPORT_SMARTPTR_LIBS) \
	$(COSUPPORT_ALLOCATORS_LIBS) \
	$(COSUPPORT_MATH_LIBS) \
	$(COSUPPORT_STRING_LIBS) \
	$(LIBSGX_LIBS) \
	$(LIBSGXUTILS_LIBS) \
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
	$(LIBSGXUTILS_DEPENDENCIES) \
	$(MAESTRO_METAMAP_DEPENDENCIES)
