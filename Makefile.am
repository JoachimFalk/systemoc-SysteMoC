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

SUBDIRS=

pkgincludeprefix=hscdmoc/

include $(top_srcdir)/jfrules.mk

compileheader-libhscdmoc.stamp: $(libhscdmoc_CH_LTOBJS)
	@date > $@

INCLUDES	= -I$(top_srcdir)/systemc-2.0.1/include		\
		  -I$(top_srcdir)/systemc-2.0.1/src		\
		  -I./include					\
		  -I$(srcdir)
AM_LDFLAGS	= -L$(top_srcdir)/systemc-2.0.1/lib-linux	\
		  -L$(builddir)/systemc-hscdmoc/lib

#noinst_SCRIPTS  = test_pathstuff.sh

EXTRA_DIST	=

#noinst_PROGRAMS = test_pathstuff

PROGLDFLAGS=

lib_LTLIBRARIES = libhscdmoc.la
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
	hscd_structure.hpp		\
	commondefs.h

noinst_HEADERS=	oneof.hpp

libhscdmoc_CH_LTOBJS=			\
	$(hscd_op_hpp_CH_LTOBJS)	\
	$(hscd_pggen_hpp_CH_LTOBJS)

libhscdmoc_la_LDFLAGS = -version-info 0:0:0 -no-undefined -lsystemc -lstdc++
libhscdmoc_la_LIBADD = $(libhscdmoc_CH_LTOBJS)
libhscdmoc_la_DEPENDENCIES = compileheader-libhscdmoc.stamp include

libhscdmoc_la_SOURCES =			\
	hscd_structure.cpp		\
	hscd_root_node.cpp		\
	hscd_pggen.cpp			\
	hscd_root_if.cpp

# Need pkinclude files in include/$(pkgincludeprefix)
$(libhscdmoc_la_OBJECTS) $(libhscdmoc_CH_LTOBJS): include

#test_pathstuff_LDADD = libhscdmoc.la
#test_pathstuff_SOURCES = test_pathstuff.C
