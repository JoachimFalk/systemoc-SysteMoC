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

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

SUBDIRS= @SUBDIRS@

AM_LDFLAGS = @AM_LDFLAGS@ $(SYSTEMOC_LDFLAGS)
AM_CPPFLAGS= @AM_CPPFLAGS@ $(SYSTEMOC_INCLUDE) -Wno-deprecated

noinst_PROGRAMS=		\
	hello			\
	src-sink		\
	guards			\
	rates			\
	data-flow		\
	ping			\
	mini-filter		\
	vpc-src-sink		\
	vpc-src-sink-config-api

if PKG_SYSTEMOC_USE_SRCDIR_VERSION
SYSTEMOC_LINKFLAGS = $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
SYSTEMOC_LINKFLAGS = -lsystemoc
endif

hello_SOURCES		= hello-systemoc.cpp
hello_LDADD  		= $(SYSTEMOC_LINKFLAGS)

src_sink_SOURCES	= source-sink.cpp
src_sink_LDADD		= $(SYSTEMOC_LINKFLAGS)

guards_SOURCES		= guards.cpp
guards_LDADD		= $(SYSTEMOC_LINKFLAGS)

rates_SOURCES		= rates.cpp
rates_LDADD		= $(SYSTEMOC_LINKFLAGS)

data_flow_SOURCES	= data-flow.cpp
data_flow_LDADD		= $(SYSTEMOC_LINKFLAGS)

ping_SOURCES		= ping.cpp
ping_LDADD		= $(SYSTEMOC_LINKFLAGS)

mini_filter_SOURCES	= mini-filter.cpp
mini_filter_LDADD	= $(SYSTEMOC_LINKFLAGS)

vpc_src_sink_SOURCES	= vpc-source-sink.cpp
vpc_src_sink_LDADD	= $(SYSTEMOC_LINKFLAGS)

vpc_src_sink_config_api_SOURCES	= vpc-source-sink-config-api.cpp
vpc_src_sink_config_api_LDADD	= $(SYSTEMOC_LINKFLAGS)
