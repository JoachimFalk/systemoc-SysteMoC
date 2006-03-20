## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
root_srcdir=$(top_srcdir)/..
root_builddir=$(top_builddir)/..

SUBDIRS=

INCLUDES = @SYSTEMOC_INCLUDE@ -I$(srcdir)

AM_LDFLAGS = @SYSTEMOC_LDFLAGS@
AM_CPPFLAGS=-Wno-deprecated

LIBS=-lsystemoc

noinst_PROGRAMS=     sim_event single_transition td_sim  till_test

td_sim_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
td_sim_SOURCES= \
	td_sim.cpp

sim_event_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
sim_event_SOURCES= \
	smoc_event_test.cpp

till_test_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
till_test_SOURCES= \
	till_test.cpp

single_transition_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
single_transition_SOURCES= \
	single_transition.cpp
