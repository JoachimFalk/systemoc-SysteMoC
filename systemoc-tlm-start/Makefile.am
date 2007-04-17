root_srcdir   := $(shell cd $(top_srcdir)/.. && pwd)
root_builddir := $(shell cd $(top_builddir)/.. && pwd)

include $(auxdir)/jfrules.mk

SUBDIRS=

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@ -Wno-deprecated -I$(srcdir)/tlm

LIBS= @LIBS@ -lsystemoc

noinst_PROGRAMS=systemoc_test bus_test sqr_hierachic

bus_test_SOURCES=bus_test.cpp
bus_test_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@

systemoc_test_SOURCES=systemoc_test.cpp
systemoc_test_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@

sqr_hierachic_SOURCES=sqr-hierachic.cpp
sqr_hierachic_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
