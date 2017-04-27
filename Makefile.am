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

AM_CPPFLAGS= @AM_CPPFLAGS@ \
	@SYSTEMOC_INCLUDE@

AM_LDFLAGS = @AM_LDFLAGS@ \
	@SYSTEMOC_LDFLAGS@

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

hello_LDADD  		= $(SYSTEMOC_LIBS)
hello_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
hello_SOURCES		= hello-systemoc.cpp

src_sink_LDADD		= $(SYSTEMOC_LIBS)
src_sink_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
src_sink_SOURCES	= source-sink.cpp

guards_LDADD		= $(SYSTEMOC_LIBS)
guards_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
guards_SOURCES		= guards.cpp

rates_LDADD		= $(SYSTEMOC_LIBS)
rates_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
rates_SOURCES		= rates.cpp

data_flow_LDADD		= $(SYSTEMOC_LIBS)
data_flow_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
data_flow_SOURCES	= data-flow.cpp

ping_LDADD		= $(SYSTEMOC_LIBS)
ping_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
ping_SOURCES		= ping.cpp

mini_filter_LDADD	= $(SYSTEMOC_LIBS)
mini_filter_DEPENDENCIES= $(SYSTEMOC_DEPENDENCIES)
mini_filter_SOURCES	= mini-filter.cpp

vpc_src_sink_LDADD		= $(SYSTEMOC_LIBS)
vpc_src_sink_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
vpc_src_sink_SOURCES		= vpc-source-sink.cpp

vpc_src_sink_config_api_LDADD		= $(SYSTEMOC_LIBS)
vpc_src_sink_config_api_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
vpc_src_sink_config_api_SOURCES		= vpc-source-sink-config-api.cpp
