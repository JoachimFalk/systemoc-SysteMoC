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

SUBDIRS=

if SYNTH
  include $(root_builddir)/Synthesis-SW/Rules-Synth.mk
endif

INCLUDES = @INCLUDES@ @SYSTEMOC_INCLUDE@

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@
AM_CPPFLAGS=-Wno-deprecated

LIBS=-lsystemoc

noinst_PROGRAMS=		\
	simulation-mpeg4	\
	simulation-test-idct1d	\
	simulation-test-idct2d	\
	simulation-test-parser

simulation_mpeg4_SOURCES = design-mpeg4.cpp
simulation_mpeg4_DEPENDENCIES =	@SYSTEMOC_DEPENDENCIES@		

simulation_test_idct2d_SOURCES= design-test-idct2d.cpp
simulation_test_idct2d_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@

simulation_test_idct1d_SOURCES= design-test-idct1d.cpp
simulation_test_idct1d_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@

simulation_test_parser_SOURCES= test_parser.cpp
simulation_test_parser_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@


all: test_in.dat

%.dat: %-dat
	cp $^ $@

if SYNTH
SYNTHESIS_SUBDIR = synthesis

$(SYNTHESIS_SUBDIR)/%.dat: %-dat
	cp $^ $@

synth: $(SYNTHESIS_SUBDIR)/smoc-synth-test-idct2d $(SYNTHESIS_SUBDIR)/test_in.dat

$(SYNTHESIS_SUBDIR)/test-idct2d.ac.xml: $(SYNTHESIS_SUBDIR)/design-test-idct2d.ast.xml

.PHONY: synth
endif
