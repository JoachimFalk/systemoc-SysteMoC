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
root_builddir=$(top_builddir)/..

SYSTEMOC_BUILDDIR=$(root_builddir)/SysteMoC-v0.3

SUBDIRS=

INCLUDES = @SYSTEMOC_INCLUDE@ -I$(srcdir)

AM_LDFLAGS = @SYSTEMOC_LDFLAGS@
AM_CPPFLAGS=-Wno-deprecated

LIBS=-lsystemoc

noinst_PROGRAMS= simulation-select

simulation_select_SOURCES= design-select.cpp
simulation_select_DEPENDENCIES=				\
	$(SYSTEMOC_BUILDDIR)/libsystemoc.la
