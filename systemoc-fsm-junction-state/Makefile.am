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

AM_CPPFLAGS= @AM_CPPFLAGS@ \
	@SYSTEMOC_INCLUDE@

AM_LDFLAGS = @AM_LDFLAGS@ \
	@SYSTEMOC_LDFLAGS@

noinst_PROGRAMS= simulation-junction-state

simulation_junction_state_SOURCES= design-junction-state.cpp
simulation_junction_state_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
simulation_junction_state_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
simulation_junction_state_LDADD += -lsystemoc
endif

autotest.sh: simulation-junction-state $(srcdir)/autotest-sh
	rm -f $@
	echo "#!" $(BASH) > $@
	cat autovars.sh $(srcdir)/autotest-sh >> $@
	chmod +x $@

# to check we just call the 'autotest.sh' script
check-local: autotest.sh
	./autotest.sh
