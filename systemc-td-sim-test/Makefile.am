## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
root_srcdir=$(top_srcdir)/..
root_builddir=$(top_builddir)/..

SUBDIRS=

SYSTEMOC_BUILDDIR=$(root_builddir)/SysteMoC-v0.3

INCLUDES =						\
	@SYSTEMC_INCLUDE@				\
	-I$(SYSTEMOC_BUILDDIR)/include/systemoc		\
	-I$(srcdir)

AM_LDFLAGS=						\
	@SYSTEMC_LDFLAGS@				\
	-L$(SYSTEMOC_BUILDDIR)

noinst_PROGRAMS=     sim_event

AM_CPPFLAGS=-Wno-deprecated
LIBS=-lsystemc -lsystemoc

td_sim_LDFLAGS = -lsystemoc
td_sim_DEPENDENCIES = $(SYSTEMOC_BUILDDIR)/libsystemoc.la
td_sim_SOURCES= \
	td_sim.cpptd_sim_LDFLAGS = -lsystemoc


sim_event_LDFLAGS = -lsystemoc
sim_event_DEPENDENCIES = $(SYSTEMOC_BUILDDIR)/libsystemoc.la
sim_event_SOURCES= \
	smoc_event_test.cpp