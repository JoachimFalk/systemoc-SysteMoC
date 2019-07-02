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

SUBDIRS =

export PKG_CONFIG_PATH

src/Makefile: src/Makefile.sample
	@$(MKDIR_P) src && $(SED) \
	    -e 's|[@]prefix[@]|$(prefix)|g' \
	    -e 's|[@]exec_prefix[@]|$(exec_prefix)|g' \
	    -e 's|[@]libdir[@]|$(libdir)|g' \
	  < $^ > $@ || { rm -f $@ && false; }

all-local: src/Makefile
	@$(MAKE) -C src VPATH=$(abs_srcdir)/src all
clean-local:
	@if test -f src/Makefile; then \
	  $(MAKE) -C src VPATH=$(abs_srcdir)/src clean; \
	fi
	-@rm -f src/Makefile
	-@rmdir src 2>/dev/null

#CLEANFILES=src/Makefile

docdir = ${datarootdir}/doc/SysteMoC

nobase_doc_DATA = \
	src/Makefile \
	src/CMakeLists.txt \
	src/guards/cpp/guards.cpp \
	src/source-sink/cpp/source-sink.cpp \
	src/dynamics/cpp/dynamics.cpp \
	src/rates/cpp/rates.cpp \
	src/ping/cpp/ping.cpp \
	src/data-flow/cpp/data-flow.cpp \
	src/hello-systemoc/cpp/hello-systemoc.cpp \
	src/mini-filter/cpp/mini-filter.cpp
