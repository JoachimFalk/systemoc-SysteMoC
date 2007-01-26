## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
root_srcdir   := $(shell cd $(top_srcdir)/.. && pwd)
root_builddir := $(shell cd $(top_builddir)/.. && pwd)

include $(auxdir)/jfrules.mk

SUBDIRS=

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@ -Wno-deprecated

noinst_PROGRAMS=    smoc2test oneof atutest_atomic atutest_nonatomic smoc1_vs_smoc2_test memory_access_read memory_access_write

smoc2test_LDFLAGS = -lsystemoc
smoc2test_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@

smoc2test_SOURCES= \
	smoc2test.cpp tt_ib.cpp

atutest_atomic_LDFLAGS = -lsystemoc
atutest_atomic_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
atutest_atomic_SOURCES= \
	atutest_atomic.cpp tt_ib.cpp ib_m_atu_atomic.cpp

atutest_nonatomic_LDFLAGS = -lsystemoc
atutest_nonatomic_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
atutest_nonatomic_SOURCES= \
	atutest_nonatomic.cpp tt_ib.cpp ib_m_atu_nonatomic.cpp

oneof_SOURCES= \
	oneof.cpp

smoc1_vs_smoc2_test_LDFLAGS = -lsystemoc
smoc1_vs_smoc2_test_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
smoc1_vs_smoc2_test_SOURCES= \
	smoc1_vs_smoc2_test.cpp

memory_access_read_LDFLAGS = -lsystemoc
memory_access_read_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
memory_access_read_SOURCES= \
	memory_access_read_test.cpp

memory_access_write_LDFLAGS = -lsystemoc
memory_access_write_DEPENDENCIES = @SYSTEMOC_DEPENDENCIES@
memory_access_write_SOURCES= \
	memory_access_write_test.cpp
