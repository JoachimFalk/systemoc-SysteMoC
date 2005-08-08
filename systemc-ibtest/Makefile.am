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

noinst_PROGRAMS=    smoc2test oneof atutest_atomic atutest_nonatomic

AM_CPPFLAGS=-Wno-deprecated
LIBS=-lsystemc -lsystemoc

smoc2test_LDFLAGS = -lsystemoc
smoc2test_DEPENDENCIES = $(SYSTEMOC_BUILDDIR)/libsystemoc.la
smoc2test_SOURCES= \
	smoc2test.cpp tt_ib.cpp

atutest_atomic_LDFLAGS = -lsystemoc
atutest_atomic_DEPENDENCIES = $(SYSTEMOC_BUILDDIR)/libsystemoc.la
atutest_atomic_SOURCES= \
	atutest_atomic.cpp tt_ib.cpp ib_m_atu_atomic.cpp

atutest_nonatomic_LDFLAGS = -lsystemoc
atutest_nonatomic_DEPENDENCIES = $(SYSTEMOC_BUILDDIR)/libsystemoc.la
atutest_nonatomic_SOURCES= \
	atutest_nonatomic.cpp tt_ib.cpp ib_m_atu_nonatomic.cpp

oneof_SOURCES= \
	oneof.cpp
