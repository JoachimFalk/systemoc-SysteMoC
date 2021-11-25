# -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
# vim: set sw=2 ts=8 et:
# 
# Copyright (c)
#   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in

include $(auxdir)/jfrules.mk

SUBDIRS =

AUTOMAKE_OPTIONS = nostdinc

AM_CPPFLAGS = @AM_CPPFLAGS@ \
        -I$(top_builddir)
AM_LDFLAGS  = @AM_LDFLAGS@ \
	$(COSUPPORT_STRING_LDFLAGS) \
	$(COSUPPORT_STREAMS_LDFLAGS) \
	$(COSUPPORT_RANDOM_LDFLAGS) \
        $(LIBNGANALYSIS_LDFLAGS) \
        $(LIBSGXUTILS_LDFLAGS) \
	$(BOOST_LDFLAGS)

bin_PROGRAMS = \
	sng-to-dot

if SYSTEMOC_ENABLE_NGANALYSIS
bin_PROGRAMS += \
        sng-generate-sdf
endif

sng_to_dot_CPPFLAGS = $(AM_CPPFLAGS) \
	$(COSUPPORT_STRING_INCLUDE) \
	$(COSUPPORT_STREAMS_INCLUDE) \
	$(BOOST_INCLUDE) \
        -I$(top_srcdir)/src/systemoc-sng/headers
sng_to_dot_LDADD = $(AM_LDFLAGS) \
	$(COSUPPORT_STRING_LIBS) \
	$(COSUPPORT_STREAMS_LIBS) \
	-lboost_program_options$(BOOST_LIBPOSTFIX) \
	-lboost_filesystem$(BOOST_LIBPOSTFIX) \
	-lboost_system$(BOOST_LIBPOSTFIX) \
        ../libsystemoc-sng.la
sng_to_dot_DEPENDENCIES = \
	$(COSUPPORT_STRING_DEPENDENCIES) \
	$(COSUPPORT_STREAMS_DEPENDENCIES) \
        ../libsystemoc-sng.la
sng_to_dot_SOURCES = \
	src/sng-to-dot/cpp/sng-to-dot.cpp

sng_generate_sdf_CPPFLAGS = $(AM_CPPFLAGS) \
	$(COSUPPORT_STRING_INCLUDE) \
	$(COSUPPORT_STREAMS_INCLUDE) \
	$(COSUPPORT_RANDOM_INCLUDE) \
	$(LIBNGANALYSIS_INCLUDE) \
	$(LIBSGXUTILS_INCLUDE) \
	$(BOOST_INCLUDE) \
        -I$(top_srcdir)/src/systemoc-sng/headers
sng_generate_sdf_LDADD = \
	$(COSUPPORT_STRING_LIBS) \
	$(COSUPPORT_RANDOM_LIBS) \
	$(COSUPPORT_RANDOM_LIBS) \
	$(LIBNGANALYSIS_LIBS) \
	$(LIBSGXUTILS_LIBS) \
	-lboost_program_options$(BOOST_LIBPOSTFIX) \
	-lboost_filesystem$(BOOST_LIBPOSTFIX) \
        ../libsystemoc-sng.la
sng_generate_sdf_DEPENDENCIES = \
	$(COSUPPORT_STRING_DEPENDENCIES) \
	$(COSUPPORT_RANDOM_DEPENDENCIES) \
	$(LIBNGANALYSIS_DEPENDENCIES) \
	$(LIBSGXUTILS_DEPENDENCIES) \
        ../libsystemoc-sng.la
sng_generate_sdf_SOURCES = \
	src/sng-generate-sdf/cpp/sng-generate-sdf.cpp
