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

AM_CPPFLAGS = @AM_CPPFLAGS@
AM_LDFLAGS  = @AM_LDFLAGS@ \
	$(COSUPPORT_STRING_LDFLAGS) \
	$(COSUPPORT_STREAMS_LDFLAGS) \
	$(BOOST_LDFLAGS)

bin_PROGRAMS = \
	sng-to-dot
noinst_DATA =

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
