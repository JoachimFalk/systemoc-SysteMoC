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

SUBDIRS=

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

pkgincludedir=$(includedir)/systemoc

compileheader-libsystemoc.stamp: $(libsystemoc_CH_LTOBJS)
	@date > $@

AM_CPPFLAGS =					\
	@AM_CPPFLAGS@				\
	@SYSTEMC_INCLUDE@			\
	@BOOST_INCLUDE@				\
	@SYSTEMC_VPC_INCLUDE@			\
	-Ipkginclude				\
	-I$(srcdir)/pkginclude

AM_LDFLAGS=				\
	@AM_LDFLAGS@			\
	@SYSTEMC_LDFLAGS@		\
	@BOOST_LDFLAGS@			\
	@SYSTEMC_VPC_LDFLAGS@

#noinst_SCRIPTS  = test_pathstuff.sh

EXTRA_DIST	=

#noinst_PROGRAMS = test_pathstuff

PROGLDFLAGS=

lib_LTLIBRARIES = libsystemoc.la
pkginclude_HEADERS =					\
	pkginclude/systemoc/smoc_config.h		\
							\
	pkginclude/systemoc/smoc_expr.hpp		\
	pkginclude/systemoc/smoc_ast_common.hpp		\
	pkginclude/systemoc/smoc_ast_systemoc.hpp	\
	pkginclude/systemoc/smoc_fifo.hpp		\
	pkginclude/systemoc/smoc_sr_signal.hpp		\
	pkginclude/systemoc/smoc_multicast_sr_signal.hpp\
	pkginclude/systemoc/smoc_moc.hpp		\
	pkginclude/systemoc/smoc_port.hpp		\
	pkginclude/systemoc/smoc_root_node.hpp		\
	pkginclude/systemoc/smoc_firing_rules.hpp	\
	pkginclude/systemoc/smoc_guard.hpp		\
	pkginclude/systemoc/smoc_node_types.hpp		\
	pkginclude/systemoc/smoc_rendezvous.hpp		\
	pkginclude/systemoc/smoc_root_port.hpp		\
	pkginclude/systemoc/smoc_graph_type.hpp		\
	pkginclude/systemoc/smoc_pggen.hpp		\
	pkginclude/systemoc/smoc_chan_if.hpp		\
	pkginclude/systemoc/smoc_event.hpp		\
	pkginclude/systemoc/smoc_storage.hpp		\
							\
	pkginclude/systemoc/MultiHopEvent.hpp		\
	pkginclude/systemoc/hscd_tdsim_TraceLog.hpp	\
							\
	pkginclude/systemoc/hscd_op.hpp			\
	pkginclude/systemoc/hscd_port.hpp		\
	pkginclude/systemoc/hscd_root_port_list.hpp	\
	pkginclude/systemoc/hscd_node_types.hpp		\
	pkginclude/systemoc/hscd_fifo.hpp		\
	pkginclude/systemoc/hscd_structure.hpp		\
	pkginclude/systemoc/hscd_graph_type.hpp		\
	pkginclude/systemoc/hscd_scheduler.hpp		\
	pkginclude/systemoc/hscd_pggen.hpp		\
	pkginclude/systemoc/hscd_moc.hpp

if SYSTEMOC_ENABLE_WSDF
pkginclude_HEADERS +=					\
	pkginclude/systemoc/smoc_md_array_access.hpp	\
	pkginclude/systemoc/smoc_md_array.hpp		\
	pkginclude/systemoc/smoc_md_chan_if.hpp \
	pkginclude/systemoc/smoc_md_buffer.hpp		\
	pkginclude/systemoc/smoc_md_fifo.hpp		\
	pkginclude/systemoc/smoc_md_loop.hpp		\
	pkginclude/systemoc/smoc_md_port.hpp		\
	pkginclude/systemoc/smoc_vector.hpp		\
	pkginclude/systemoc/smoc_wsdf_edge.hpp \
	pkginclude/systemoc/smoc_md_ba_basic_tree.hpp \
	pkginclude/systemoc/smoc_md_buffer_analysis.hpp \
	pkginclude/systemoc/smoc_md_ba_linearized_buffer.hpp \
	pkginclude/systemoc/smoc_md_ba_linearized_buffer_size.hpp \
	pkginclude/systemoc/smoc_md_ba_linearized_buffer_schedule.hpp
endif


# noinst_HEADERS=	oneof.hpp

libsystemoc_CH_LTOBJS=				\
	$(smoc_firing_rules_hpp_CH_LTOBJS)	\
	$(smoc_pggen_hpp_CH_LTOBJS)
#	$(smoc_op_hpp_CH_LTOBJS)

if SYSTEMOC_ENABLE_VPC
SYSTEMC_VPC_LIB = -lsystemcvpc
endif

libsystemoc_la_LDFLAGS = -version-info 0:0:0 -no-undefined $(SYSTEMC_VPC_LIB) -lcosupport -lsystemc -lstdc++
libsystemoc_la_LIBADD = $(libsystemoc_CH_LTOBJS)
libsystemoc_la_DEPENDENCIES = compileheader-libsystemoc.stamp @SUPPORT_DEPENDENCIES@ @SYSTEMC_VPC_DEPENDENCIES@

libsystemoc_la_SOURCES =		\
	smoc_main.cpp			\
	smoc_ast_systemoc.cpp		\
	smoc_fifo.cpp			\
	smoc_multicast_sr_signal.cpp	\
	smoc_graph_type.cpp		\
	smoc_rendezvous.cpp		\
	smoc_root_node.cpp		\
	smoc_root_port.cpp		\
	smoc_firing_rules.cpp		\
	smoc_pggen.cpp			\
	smoc_chan_if.cpp		\
	smoc_guard.cpp			\
	smoc_moc.cpp			\
					\
	MultiHopEvent.cpp		\
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
