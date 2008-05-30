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
root_srcdir   := $(shell cd $(top_srcdir)/.. && pwd)
root_builddir := $(shell cd $(top_builddir)/.. && pwd)

include $(auxdir)/jfrules.mk
include ../BuildSystem/doxygen.am

SUBDIRS = pkginclude

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

compileheader-libsystemoc.stamp: $(libsystemoc_CH_LTOBJS)
	@date > $@

AM_CPPFLAGS =					\
	@AM_CPPFLAGS@				\
	@SYSTEMC_INCLUDE@			\
	@SUPPORT_INCLUDE@			\
	@LIBSGX_INCLUDE@			\
	@BOOST_INCLUDE@				\
	@SYSTEMC_VPC_INCLUDE@			\
	-Ipkginclude				\
	-I$(srcdir)/pkginclude

AM_LDFLAGS=				\
	@AM_LDFLAGS@			\
	@SYSTEMC_LDFLAGS@		\
	@SUPPORT_LDFLAGS@		\
	@LIBSGX_LDFLAGS@		\
	@BOOST_LDFLAGS@			\
	@SYSTEMC_VPC_LDFLAGS@

#noinst_SCRIPTS  = test_pathstuff.sh

EXTRA_DIST	=

#noinst_PROGRAMS = test_pathstuff

PROGLDFLAGS=

lib_LTLIBRARIES = libsystemoc.la

libsystemoc_CH_LTOBJS=				\
	$(smoc_firing_rules_hpp_CH_LTOBJS)	\
	$(smoc_pggen_hpp_CH_LTOBJS)

libsystemoc_la_LDFLAGS =				\
	-version-info 0:0:0 				\
	-no-undefined 					\
	-lsystemc 					\
	-lboost_program_options@BOOST_LIBPOSTFIX@
libsystemoc_la_DEPENDENCIES = 				\
	compileheader-libsystemoc.stamp
libsystemoc_la_CPPFLAGS = $(AM_CPPFLAGS)
libsystemoc_la_LIBADD = 				\
	$(libsystemoc_CH_LTOBJS)			\
	$(pkg_support_builddir)/libcosupport-systemc.la	\
	$(pkg_support_builddir)/libcosupport-streams.la	\
	$(pkg_support_builddir)/libcosupport-smartptr.la\
	$(pkg_libsgx_builddir)/libsgx.la
if SYSTEMOC_ENABLE_VPC
libsystemoc_la_LIBADD += $(pkg_systemc_vpc_builddir)/libsystemcvpc.la
endif

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
					\
	hscd_tdsim_TraceLog.cpp

if SYSTEMOC_ENABLE_WSDF
libsystemoc_la_SOURCES +=		\
	wsdf/smoc_md_buffer.cpp		\
	wsdf/smoc_md_loop.cpp		\
	wsdf/smoc_wsdf_edge.cpp		\
	wsdf/smoc_md_port.cpp \
	wsdf/smoc_md_buffer_analysis.cpp \
	wsdf/smoc_md_ba_linearized_buffer.cpp \
	wsdf/smoc_md_ba_linearized_buffer_size.cpp \
	wsdf/smoc_md_ba_linearized_buffer_schedule.cpp
endif
