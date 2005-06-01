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

SUBDIRS=
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

ac_aux_dir = @ac_aux_dir@

include $(ac_aux_dir)/jfrules.mk

pkgincludeprefix=systemoc/


compileheader-libsystemoc.stamp: $(libsystemoc_CH_LTOBJS)
	@date > $@

INCLUDES =				\
	@SYSTEMC_INCLUDE@		\
	-I./include			\
	-I$(srcdir)
#	-I$(root_srcdir)/systemc-2.0.1/src

AM_LDFLAGS = @SYSTEMC_LDFLAGS@ -lsystemc

#noinst_SCRIPTS  = test_pathstuff.sh

EXTRA_DIST	=

#noinst_PROGRAMS = test_pathstuff

PROGLDFLAGS=

lib_LTLIBRARIES = libsystemoc.la
pkginclude_HEADERS =			\
	smoc_fifo.hpp			\
	smoc_moc.hpp			\
	smoc_port.hpp			\
	smoc_root_node.hpp		\
	smoc_firing_rules.hpp		\
	smoc_node_types.hpp		\
	smoc_rendezvous.hpp		\
	smoc_root_port.hpp		\
	smoc_graph_type.hpp		\
	smoc_pggen.hpp			\
	smoc_chan_if.hpp		\
	oneof.hpp			\
	commondefs.h

# noinst_HEADERS=	oneof.hpp

libsystemoc_CH_LTOBJS=				\
	$(smoc_firing_rules_hpp_CH_LTOBJS)	\
	$(smoc_pggen_hpp_CH_LTOBJS)
#	$(smoc_op_hpp_CH_LTOBJS)

libsystemoc_la_LDFLAGS = -version-info 0:0:0 -no-undefined -lstdc++
libsystemoc_la_LIBADD = $(libsystemoc_CH_LTOBJS)
libsystemoc_la_DEPENDENCIES = compileheader-libsystemoc.stamp include

libsystemoc_la_SOURCES =		\
	smoc_fifo.cpp			\
	smoc_graph_type.cpp		\
	smoc_rendezvous.cpp		\
	smoc_root_node.cpp		\
	smoc_root_port.cpp		\
	smoc_firing_rules.cpp		\
	smoc_pggen.cpp			\
	smoc_chan_if.cpp

# Need pkinclude files in include/$(pkgincludeprefix)
$(libsystemoc_la_OBJECTS) $(libsystemoc_CH_LTOBJS): include

#noinst_PROGRAMS = bar
#
#bar_SOURCES= bar.cpp
#bar_LDFLAGS= @SYSTEMC_LDFLAGS@

#test_pathstuff_LDADD = libsystemoc.la
#test_pathstuff_SOURCES = test_pathstuff.C
