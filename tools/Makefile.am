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

SUBDIRS = .

AUTOMAKE_OPTIONS = nostdinc

AM_CPPFLAGS=				\
	@AM_CPPFLAGS@			\
	$(COSUPPORT_BASE_INCLUDE)	\
	$(COSUPPORT_STRING_INCLUDE)	\
	$(COSUPPORT_MATH_INCLUDE)	\
	$(COSUPPORT_STREAMS_INCLUDE)	\
	$(COSUPPORT_XERCES_INCLUDE)	\
	$(COSUPPORT_RANDOM_INCLUDE)	\
	$(LIBNGANALYSIS_INCLUDE)	\
	$(BOOST_INCLUDE)		\
	$(XERCES_INCLUDE)

AM_LDFLAGS=				\
	@AM_LDFLAGS@			\
	$(COSUPPORT_BASE_LDFLAGS)	\
	$(COSUPPORT_STRING_LDFLAGS)	\
	$(COSUPPORT_MATH_LDFLAGS)	\
	$(COSUPPORT_STREAMS_LDFLAGS)	\
	$(COSUPPORT_XERCES_LDFLAGS)	\
	$(COSUPPORT_RANDOM_LDFLAGS)	\
	$(LIBNGANALYSIS_LDFLAGS)	\
	$(BOOST_LDFLAGS)		\
	$(XERCES_LDFLAGS)

CLEANFILES=

BUILT_SOURCES=src/sng-to-dot/resources/sng-xsd.c

#%-xsd.c: %.xsd Makefile
%-xsd.c: %.xsd
	test -d  $(dir $*) || mkdir -p $(dir $*)
	{ set -e; echo "static char const $(notdir $*)XSD[] ="; \
	  sed -e 's/\(["\\]\)/\\\1/g' -e 's/^\(.*\)$$/  "\1\\n"/' $<; \
	  echo ";"; \
	} > $@
#%-dtd.c: %.dtd Makefile
%-dtd.c: %.dtd
	test -d  $(dir $*) || mkdir -p $(dir $*)
	{ set -e; echo "static char const $(notdir $*)DTD[] ="; \
	  sed -e 's/\(["\\]\)/\\\1/g' -e 's/^\(.*\)$$/  "\1\\n"/' $<; \
	  echo ";"; \
	} > $@

CLEANFILES += $(BUILT_SOURCES)

pkgdatadir = $(datadir)/$(PACKAGE)-$(PACKAGE_VERSION)

bin_PROGRAMS = 				\
	sng-to-dot
noinst_DATA =

sng_to_dot_SOURCES = \
	src/sng-to-dot/cpp/sng-to-dot.cpp
sng_to_dot_CPPFLAGS = $(AM_CPPFLAGS) \
	-I src/sng-to-dot/resources
sng_to_dot_LDADD = \
	$(COSUPPORT_STRING_LIBS) \
	$(COSUPPORT_XERCES_LIBS) \
	$(COSUPPORT_STREAMS_LIBS) \
	-lboost_program_options$(BOOST_LIBPOSTFIX) \
	-lboost_filesystem$(BOOST_LIBPOSTFIX) \
	-lboost_system$(BOOST_LIBPOSTFIX)
sng_to_dot_DEPENDENCIES = \
	$(COSUPPORT_STRING_DEPENDENCIES)
