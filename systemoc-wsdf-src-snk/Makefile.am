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

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@ -L/local/lib
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@ -Wno-deprecated -Dcimg_display_type=0 
# -Dcimg_use_magick

noinst_PROGRAMS= simulation-wsdf-src-snk \
	simulation-1D-src-snk \
	plain-cpp

simulation_wsdf_src_snk_SOURCES= design-wsdf-src-snk.cpp
simulation_wsdf_src_snk_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
simulation_wsdf_src_snk_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
simulation_wsdf_src_snk_LDADD += -lsystemoc
endif

simulation_1D_src_snk_SOURCES= design1D-src-snk.cpp
simulation_1D_src_snk_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
simulation_1D_src_snk_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
simulation_1D_src_snk_LDADD += -lsystemoc
endif

plain_cpp_SOURCES= plain_cpp.cpp
