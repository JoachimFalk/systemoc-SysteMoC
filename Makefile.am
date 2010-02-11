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
	$(SYSTEMC_VPC_INCLUDE)		\
	$(BOOST_INCLUDE)		\
	-Ipkginclude			\
	-I$(srcdir)/pkginclude

AM_LDFLAGS=				\
	@AM_LDFLAGS@			\
	$(SYSTEMC_LDFLAGS)		\
	$(TLM1_LDFLAGS)			\
	$(XERCES_LDFLAGS)		\
	$(COSUPPORT_LDFLAGS)		\
	$(LIBWSDF_LDFLAGS)		\
	$(LIBSGX_LDFLAGS)		\
	$(SYSTEMC_VPC_INCLUDE)		\
	$(BOOST_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = SysteMoC.pc

lib_LTLIBRARIES = libsystemoc.la

libsystemoc_la_SOURCES =			\
	smoc-core/IdPool.cpp			\
	smoc-core/smoc_simulation_ctx.cpp	\
	smoc-core/smoc_hooking.cpp		\
	smoc-core/smoc_main.cpp			\
	smoc-core/smoc_elab_and_sim.cpp		\
	smoc-core/smoc_fifo.cpp			\
	smoc-core/smoc_multiplex_fifo.cpp	\
	smoc-core/smoc_multireader_fifo.cpp	\
	smoc-core/smoc_latency_queues.cpp	\
	smoc-core/smoc_multicast_sr_signal.cpp	\
	smoc-core/smoc_reset.cpp		\
	smoc-core/smoc_graph_type.cpp		\
	smoc-core/smoc_graph_tt.cpp		\
	smoc-core/smoc_actor.cpp		\
	smoc-core/smoc_root_node.cpp		\
	smoc-core/smoc_sysc_port.cpp		\
	smoc-core/smoc_firing_rules.cpp		\
	smoc-core/smoc_chan_if.cpp		\
	smoc-core/smoc_root_chan.cpp		\
	smoc-core/smoc_moc.cpp			\
	smoc-core/SMXDumper.cpp			\
	smoc-core/smoc_graph_synth.cpp		\
	smoc-core/smoc_func_call.cpp		\
	smoc-core/smoc_debug_stream.cpp		\
						\
	smoc-core/hscd_tdsim_TraceLog.cpp
#	smoc_ngx_sync.cpp

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
	$(AM_LDFLAGS)						\
	-version-info 0:0:0 					\
	-no-undefined
libsystemoc_la_LIBADD = 					\
	-lsystemc 						\
	-lboost_program_options$(BOOST_LIBPOSTFIX)
if PKG_COSUPPORT_USE_SRCDIR_VERSION
libsystemoc_la_LIBADD += 					\
	$(COSUPPORT_LIBPATH)/libcosupport-systemc.la		\
	$(COSUPPORT_LIBPATH)/libcosupport-streams.la		\
	$(COSUPPORT_LIBPATH)/libcosupport-smartptr.la		\
	$(COSUPPORT_LIBPATH)/libcosupport-math.la
else
# It may not be possible to find COSUPPORT_LIBPATH for
# cosupport installed in the wild and not in the source dir.
libsystemoc_la_LIBADD += 					\
	-lcosupport-systemc					\
	-lcosupport-streams					\
	-lcosupport-smartptr					\
	-lcosupport-math
endif
if SYSTEMOC_ENABLE_SGX
if PKG_LIBSGX_USE_SRCDIR_VERSION
libsystemoc_la_LIBADD += $(LIBSGX_LIBPATH)/libsgx.la
else
# It may not be possible to find LIBSGX_LIBPATH for
# libsgx installed in the wild and not in the source dir.
libsystemoc_la_LIBADD += -lsgx
endif
endif
if SYSTEMOC_ENABLE_WSDF
if PKG_LIBWSDF_USE_SRCDIR_VERSION
libsystemoc_la_LIBADD += $(LIBWSDF_LIBPATH)/libwsdf.la
else
# It may not be possible to find LIBWSDF_LIBPATH for
# libwsdf installed in the wild and not in the source dir.
libsystemoc_la_LIBADD += -lwsdf
endif
endif
if SYSTEMOC_ENABLE_VPC
if PKG_SYSTEMC_VPC_USE_SRCDIR_VERSION
libsystemoc_la_LIBADD += $(SYSTEMC_VPC_LIBPATH)/libsystemcvpc.la
else
# It may not be possible to find SYSTEMC_VPC_LIBPATH for
# libsystemcvpc installed in the wild and not in the source dir.
libsystemoc_la_LIBADD += -lsystemcvpc
endif
endif
