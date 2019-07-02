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

SUBDIRS =

#AUTOMAKE_OPTIONS = nostdinc
#
#AM_CPPFLAGS= @AM_CPPFLAGS@ \
#	-Isrc/systemoc/headers \
#	-I$(srcdir)/src/systemoc/headers \
#	$(BOOST_CPPFLAGS) \
#	$(SYSTEMC_CPPFLAGS) \
#	$(TLM1_CPPFLAGS) \
#	$(COSUPPORT_SYSTEMC_CPPFLAGS) \
#	$(COSUPPORT_TRACING_CPPFLAGS) \
#	$(COSUPPORT_STREAMS_CPPFLAGS) \
#	$(COSUPPORT_SMARTPTR_CPPFLAGS) \
#	$(COSUPPORT_ALLOCATORS_CPPFLAGS) \
#	$(COSUPPORT_MATH_CPPFLAGS) \
#	$(COSUPPORT_STRING_CPPFLAGS) \
#	$(LIBSGX_CPPFLAGS) \
#	$(LIBSGXUTILS_CPPFLAGS) \
#	$(MAESTRO_METAMAP_CPPFLAGS)
#
#AM_LDFLAGS = @AM_LDFLAGS@ \
#	$(BOOST_LDFLAGS) \
#	$(SYSTEMC_LDFLAGS) \
#	$(TLM1_LDFLAGS) \
#	$(COSUPPORT_SYSTEMC_LDFLAGS) \
#	$(COSUPPORT_TRACING_LDFLAGS) \
#	$(COSUPPORT_STREAMS_LDFLAGS) \
#	$(COSUPPORT_SMARTPTR_LDFLAGS) \
#	$(COSUPPORT_ALLOCATORS_LDFLAGS) \
#	$(COSUPPORT_MATH_LDFLAGS) \
#	$(COSUPPORT_STRING_LDFLAGS) \
#	$(LIBSGX_LDFLAGS) \
#	$(LIBSGXUTILS_LDFLAGS) \
#	$(MAESTRO_METAMAP_LDFLAGS)
#
#pkgconfigdir = $(libdir)/pkgconfig
#
#nodist_pkgconfig_DATA =	\
#	libsystemoc.pc
#
#%.pc: pkgconfig/%.pc
#	@sed -e '/^srctreemode_\w\+\s*=/d' \
#	     -e 's/\$${srctreemode_libdir}\/lib\([^ 	]*\).la/-l\1/g' \
#	     -e 's/\$${srctreemode_\(\w\+\)}/\$${\1}/g' \
#	  < $^ > $@
#
#CLEANFILES=$(nodist_pkgconfig_DATA)
#
#noinst_PROGRAMS=		\
#	hello			\
#	src-sink		\
#	guards			\
#	rates			\
#	data-flow		\
#	ping			\
#	mini-filter
#
#hello_LDADD  		= libsystemoc.la
#hello_SOURCES		= src/tutorial/cpp/hello-systemoc.cpp
#
#src_sink_LDADD		= libsystemoc.la
#src_sink_SOURCES	= src/tutorial/cpp/source-sink.cpp
#
#guards_LDADD		= libsystemoc.la
#guards_SOURCES		= src/tutorial/cpp/guards.cpp
#
#rates_LDADD		= libsystemoc.la
#rates_SOURCES		= src/tutorial/cpp/rates.cpp
#
#data_flow_LDADD		= libsystemoc.la
#data_flow_SOURCES	= src/tutorial/cpp/data-flow.cpp
#
#ping_LDADD		= libsystemoc.la
#ping_SOURCES		= src/tutorial/cpp/ping.cpp
#
#mini_filter_LDADD	= libsystemoc.la
#mini_filter_SOURCES	= src/tutorial/cpp/mini-filter.cpp

export PKG_CONFIG_PATH

src/Makefile: src/Makefile.sample
	@$(MKDIR_P) src && $(SED) \
	    -e 's|[@]prefix[@]|$(prefix)|g' \
	    -e 's|[@]exec_prefix[@]|$(exec_prefix)|g' \
	    -e 's|[@]libdir[@]|$(libdir)|g' \
	  < $^ > $@ || { rm -f $@ && false; }

all-local: src/Makefile
	@$(MAKE) -C src VPATH=$(abs_srcdir)/src all
clean-local:
	@if test -f src/Makefile; then \
	  $(MAKE) -C src VPATH=$(abs_srcdir)/src clean; \
	fi
	-@rm -f src/Makefile
	-@rmdir src 2>/dev/null

#CLEANFILES=src/Makefile

docdir = ${datarootdir}/doc/SysteMoC

nobase_doc_DATA = \
	src/Makefile \
	src/CMakeLists.txt \
	src/guards/cpp/guards.cpp \
	src/source-sink/cpp/source-sink.cpp \
	src/dynamics/cpp/dynamics.cpp \
	src/rates/cpp/rates.cpp \
	src/ping/cpp/ping.cpp \
	src/data-flow/cpp/data-flow.cpp \
	src/hello-systemoc/cpp/hello-systemoc.cpp \
	src/mini-filter/cpp/mini-filter.cpp
