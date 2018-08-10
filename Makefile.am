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

SUBDIRS=src/actorlibrary/headers

AUTOMAKE_OPTIONS = nostdinc
# This is only needed if there is a configure.ac in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

pkgconfigdir = $(libdir)/pkgconfig

nodist_pkgconfig_DATA =	\
	libactorlibrary.pc

%.pc: pkgconfig/%.pc
	@sed -e '/^srctreemode_\w\+\s*=/d' \
	     -e 's/\$${srctreemode_libdir}\/lib\([^ 	]*\).la/-l\1/g' \
	     -e 's/\$${srctreemode_\(\w\+\)}/\$${\1}/g' \
	  < $^ > $@

lib_LTLIBRARIES = \
	libactorlibrary.la

libactorlibrary_la_SOURCES = \
	src/actorlibrary/cpp/TT.cpp
libactorlibrary_la_DEPENDENCIES = \
	$(SYSTEMOC_DEPENDENCIES)
libactorlibrary_la_CPPFLAGS = $(AM_CPPFLAGS) \
	$(SYSTEMOC_INCLUDE) \
	-I$(builddir)/src/actorlibrary/cpp \
	-I$(srcdir)/src/actorlibrary/headers
libactorlibrary_la_LDFLAGS = $(AM_LDFLAGS) \
	$(filter-out %/libactorlibrary.la, $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --static --libs libactorlibrary)) \
	-Wl,--no-undefined

CLEANFILES=$(nodist_pkgconfig_DATA)
