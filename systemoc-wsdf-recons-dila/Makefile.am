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

SUBDIRS=

if SYNTH
  include $(root_builddir)/Synthesis-SW/Rules-Synth.mk
endif

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@ -L/usr/X11R6/lib
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@ -Wno-deprecated

LIBS= @LIBS@ -lsystemoc -lX11

noinst_PROGRAMS= simulation-wsdf-recons-dila

simulation_wsdf_recons_dila_SOURCES= \
	design-wsdf-recons-dila.cpp
#	test_input_switch.cpp

simulation_wsdf_recons_dila_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@

all: example.bmp seed.bmp

%.bmp: %-bmp
	cp $^ $@

if SYNTH
SYNTHESIS_SUBDIR = synthesis

synth: $(SYNTHESIS_SUBDIR)/smoc-synth-wsdf

$(SYNTHESIS_SUBDIR)/wsdf.ac.xml: $(SYNTHESIS_SUBDIR)/design-wsdf.ast.xml

.PHONY: synth
endif
