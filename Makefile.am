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

root_srcdir=$(top_srcdir)/..
root_builddir = $(top_builddir)/..

SUBDIRS=
SYSTEMCVPC_BUILDDIR=$(root_builddir)/SystemC-VPC

ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

ac_aux_dir = @ac_aux_dir@

include $(ac_aux_dir)/jfrules.mk

pkgincludeprefix=systemoc/

compileheader-libsystemoc.stamp: $(libsystemoc_CH_LTOBJS)
	@date > $@

INCLUDES =					\
	@SYSTEMC_INCLUDE@			\
	-I./include				\
	-I$(srcdir)				\
	-I$(SYSTEMCVPC_BUILDDIR)/include/systemcvpc

AM_LDFLAGS=					\
	@SYSTEMC_LDFLAGS@			\
	-L$(SYSTEMCVPC_BUILDDIR)

#noinst_SCRIPTS  = test_pathstuff.sh

EXTRA_DIST	=

#noinst_PROGRAMS = test_pathstuff

PROGLDFLAGS=

lib_LTLIBRARIES = libsystemoc.la
pkginclude_HEADERS =			\
	hscd_fifo.hpp			\
	hscd_node_types.hpp		\
	hscd_op.hpp			\
	hscd_pggen.hpp			\
	hscd_port.hpp			\
	hscd_rendezvous.hpp		\
	hscd_root_if.hpp		\
	hscd_root_node.hpp		\
	hscd_root_port.hpp		\
	hscd_root_port_list.hpp		\
	hscd_scheduler.hpp		\
	hscd_moc.hpp			\
	hscd_structure.hpp		\
	hscd_graph_type.hpp		\
	oneof.hpp			\
	commondefs.h

# noinst_HEADERS=	oneof.hpp

libsystemoc_CH_LTOBJS=			\
	$(hscd_op_hpp_CH_LTOBJS)	\
	$(hscd_pggen_hpp_CH_LTOBJS)

libsystemoc_la_LDFLAGS = -version-info 0:0:0 -no-undefined @SYSTEMC_LDFLAGS@ -lsystemc -lsystemcvpc -lstdc++
libsystemoc_la_LIBADD = $(libsystemoc_CH_LTOBJS)
libsystemoc_la_DEPENDENCIES = compileheader-libsystemoc.stamp include $(SYSTEMCVPC_BUILDDIR)/libsystemcvpc.la

libsystemoc_la_SOURCES =		\
	hscd_graph_type.cpp		\
	hscd_root_node.cpp		\
	hscd_pggen.cpp			\
	hscd_root_if.cpp		\
	hscd_rendezvous.cpp		\
	hscd_fifo.cpp

# Need pkinclude files in include/$(pkgincludeprefix)
$(libsystemoc_la_OBJECTS) $(libsystemoc_CH_LTOBJS): include

#test_pathstuff_LDADD = libsystemoc.la
#test_pathstuff_SOURCES = test_pathstuff.C
