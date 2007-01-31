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

pkgincludeprefix=systemoc/

compileheader-libsystemoc.stamp: $(libsystemoc_CH_LTOBJS)
	@date > $@

AM_CPPFLAGS =				\
	@AM_CPPFLAGS@			\
	@SYSTEMC_INCLUDE@		\
	@BOOST_INCLUDE@			\
	@SYSTEMC_VPC_INCLUDE@		\
	-I./include			\
	-I$(srcdir)/wsdf		\
	-I$(srcdir)

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
pkginclude_HEADERS =			\
	smoc_expr.hpp			\
	smoc_expr.hpp			\
	smoc_fifo.hpp			\
	smoc_moc.hpp			\
	smoc_port.hpp			\
	smoc_root_node.hpp		\
	smoc_firing_rules.hpp		\
	smoc_guard.hpp			\
	smoc_node_types.hpp		\
	smoc_rendezvous.hpp		\
	smoc_root_port.hpp		\
	smoc_graph_type.hpp		\
	smoc_pggen.hpp			\
	smoc_chan_if.hpp		\
	smoc_event.hpp			\
	smoc_storage.hpp		\
					\
	hscd_tdsim_TraceLog.hpp		\
					\
	hscd_op.hpp			\
	hscd_port.hpp			\
	hscd_root_port_list.hpp		\
	hscd_node_types.hpp		\
	hscd_fifo.hpp			\
	hscd_structure.hpp		\
	hscd_graph_type.hpp		\
	hscd_scheduler.hpp		\
	hscd_pggen.hpp			\
	hscd_moc.hpp

if WSDF
pkginclude_HEADERS +=			\
	wsdf/smoc_md_array_access.hpp	\
	wsdf/smoc_md_array.hpp		\
	wsdf/smoc_md_buffer.hpp		\
	wsdf/smoc_md_fifo.hpp		\
	wsdf/smoc_md_loop.hpp		\
	wsdf/smoc_md_port.hpp		\
	wsdf/smoc_vector.hpp		\
	wsdf/smoc_wsdf_edge.hpp
endif


# noinst_HEADERS=	oneof.hpp

libsystemoc_CH_LTOBJS=				\
	$(smoc_firing_rules_hpp_CH_LTOBJS)	\
	$(smoc_pggen_hpp_CH_LTOBJS)
#	$(smoc_op_hpp_CH_LTOBJS)

libsystemoc_la_LDFLAGS = -version-info 0:0:0 -no-undefined -lsystemc -lcosupport -lstdc++
libsystemoc_la_LIBADD = $(libsystemoc_CH_LTOBJS)
libsystemoc_la_DEPENDENCIES = compileheader-libsystemoc.stamp include @pkg_support_builddir@/libcosupport.la @SYSTEMC_VPC_DEPENDENCIES@

libsystemoc_la_SOURCES =		\
	smoc_main.cpp			\
	smoc_expr.cpp			\
	smoc_fifo.cpp			\
	smoc_graph_type.cpp		\
	smoc_rendezvous.cpp		\
	smoc_root_node.cpp		\
	smoc_root_port.cpp		\
	smoc_firing_rules.cpp		\
	smoc_port.cpp			\
	smoc_pggen.cpp			\
	smoc_chan_if.cpp		\
	smoc_moc.cpp			\
					\
	hscd_tdsim_TraceLog.cpp

if WSDF
libsystemoc_la_SOURCES +=		\
	wsdf/smoc_md_buffer.cpp		\
	wsdf/smoc_md_loop.cpp		\
	wsdf/smoc_wsdf_edge.cpp		\
	wsdf/smoc_md_port.cpp
endif

# Need pkinclude files in include/$(pkgincludeprefix)
$(libsystemoc_la_OBJECTS) $(libsystemoc_CH_LTOBJS): include
