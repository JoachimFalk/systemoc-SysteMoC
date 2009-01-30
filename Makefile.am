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

SUBDIRS = pkginclude

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

AM_CPPFLAGS =				\
	@AM_CPPFLAGS@			\
	$(SYSTEMC_INCLUDE)		\
	$(TLM1_INCLUDE)			\
	$(XERCES_INCLUDE)		\
	$(COSUPPORT_INCLUDE)		\
	$(LIBWSDF_INCLUDE)		\
	$(LIBSGX_INCLUDE)		\
	$(BOOST_INCLUDE)		\
	$(SYSTEMC_VPC_INCLUDE)		\
	-Ipkginclude			\
	-I$(srcdir)/pkginclude

AM_LDFLAGS=				\
	@AM_LDFLAGS@			\
	$(SYSTEMC_LDFLAGS)		\
	$(TLM1_LDFLAGS)			\
	$(XERCES_LDFLAGS)		\
	$(COSUPPORT_LDFLAGS)		\
	$(LIBSGX_LDFLAGS)		\
	$(BOOST_LDFLAGS)

lib_LTLIBRARIES = libsystemoc.la

libsystemoc_la_SOURCES =		\
	smoc_main.cpp			\
	smoc_elab_and_sim.cpp		\
	smoc_ast_systemoc.cpp		\
	smoc_fifo.cpp			\
	smoc_multiplex_fifo.cpp		\
	smoc_multireader_fifo.cpp	\
	smoc_latency_queues.cpp		\
	smoc_multicast_sr_signal.cpp	\
	smoc_graph_type.cpp		\
	smoc_root_node.cpp		\
	smoc_root_port.cpp		\
	smoc_sysc_port.cpp		\
	smoc_firing_rules.cpp		\
	smoc_pggen.cpp			\
	smoc_chan_if.cpp		\
	smoc_root_chan.cpp		\
	smoc_guard.cpp			\
	smoc_moc.cpp			\
	smoc_ngx_sync.cpp		\
	smoc_graph_synth.cpp		\
	smoc_func_call.cpp		\
					\
	hscd_tdsim_TraceLog.cpp

if SYSTEMOC_ENABLE_WSDF
libsystemoc_la_SOURCES +=				\
	wsdf/smoc_md_buffer.cpp				\
	wsdf/smoc_md_loop.cpp				\
	wsdf/smoc_md_port.cpp 				\
	wsdf/smoc_md_buffer_analysis.cpp 		\
	wsdf/smoc_md_ba_linearized_buffer.cpp	 	\
	wsdf/smoc_md_ba_linearized_buffer_size.cpp 	\
	wsdf/smoc_md_ba_linearized_buffer_schedule.cpp
endif

libsystemoc_la_LDFLAGS =					\
	-version-info 0:0:0 					\
	-no-undefined
libsystemoc_la_LIBADD = 					\
	-lsystemc 						\
	-lboost_program_options$(BOOST_LIBPOSTFIX)		\
	@LIBSGX_LIBPATH@/libsgx.la				\
	@COSUPPORT_LIBPATH@/libcosupport-systemc.la		\
	@COSUPPORT_LIBPATH@/libcosupport-streams.la		\
	@COSUPPORT_LIBPATH@/libcosupport-smartptr.la

if SYSTEMOC_ENABLE_WSDF
libsystemoc_la_LIBADD +=					\
	@LIBWSDF_LDFLAGS@					\
	@LIBWSDF_LIBPATH@/libwsdf.la
endif

if SYSTEMOC_ENABLE_VPC
libsystemoc_la_LIBADD +=					\
	@SYSTEMC_VPC_LDFLAGS@					\
	@SYSTEMC_VPC_LIBPATH@/libsystemcvpc.la
endif

