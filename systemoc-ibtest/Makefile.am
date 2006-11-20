## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
# Erlangen-Nuremberg. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify it under
#   the terms of the GNU General Public License as published by the Free Software
#   Foundation; either version 2 of the License, or (at your option) any later
#   version.
# 
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
#   details.
# 
#   You should have received a copy of the GNU General Public License along with
#   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
#   Place, Suite 330, Boston, MA 02111-1307 USA.
# 
# --- This software and any associated documentation is provided "as is" 
# 
# IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
# BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
# CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
# DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
# NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
# DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
# HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
# ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
# ENHANCEMENTS, OR MODIFICATIONS.

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
