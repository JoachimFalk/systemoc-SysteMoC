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

SUBDIRS=

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@ -Wno-deprecated

noinst_PROGRAMS= sr-and sr-phil-simple sr-PLMON sr-multicast-test sr-multicast-hier

sr_and_SOURCES= design-sr-and.cpp
sr_and_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
sr_and_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
sr_and_LDADD += -lsystemoc
endif

sr_phil_simple_SOURCES= design-sr-phil-simple.cpp
sr_phil_simple_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
sr_phil_simple_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
sr_phil_simple_LDADD += -lsystemoc
endif

sr_PLMON_SOURCES= design-PLMON.cpp
sr_PLMON_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
sr_PLMON_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
sr_PLMON_LDADD += -lsystemoc
endif

sr_multicast_test_SOURCES= design-mc-test.cpp
sr_multicast_test_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
sr_multicast_test_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
sr_multicast_test_LDADD += -lsystemoc
endif

sr_multicast_hier_SOURCES= design-mc-hier.cpp
sr_multicast_hier_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
sr_multicast_hier_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
sr_multicast_hier_LDADD += -lsystemoc
endif
